#include "PriceManager.h"
#include "Price.h"
#include "currency.h"
#include "prices/logging.h"
#include "PriceSource.h"

#include <QTimer>
#include <QUrl>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace {
    static const int POLLING_TIMEOUT_SECONDS = 30;
    static const int DEFAULT_REFRESH_PERIOD_MILLISECONDS = 15 * 1000;
}

PriceManager * PriceManager::m_instance = nullptr;

PriceManager *PriceManager::instance(QNetworkAccessManager *networkAccessManager)
{
    if (!m_instance) {
        qDebug() << "Creating new static PriceManger instance (regular qdebug)";
        qCDebug(logPriceManager) << "Creating new static PriceManger instance";
        m_instance = new PriceManager(networkAccessManager);
    }

    return m_instance;
}

PriceManager::PriceManager(QNetworkAccessManager *manager, QObject *parent) :
    QObject(parent),
    m_manager(manager),
    m_currentPrice(nullptr),
    m_currentPriceSource(nullptr),
    m_currentCurrency(nullptr),
    m_priceSourcesAvailableModel(nullptr),
    m_timer(nullptr)
{
    qDebug() << "instantiating pricemanager debug";
    qCDebug(logPriceManager) << "Instantiating PriceManager";
    m_running = false;
    m_refreshing = false;
    m_currentPrice = new Price(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(runPriceRefresh()));

    // DEBUGGING TIME
    m_currentPriceSource = PriceSources::CoinMarketCap;
    m_currentCurrency = Currencies::USD;

    start();
}

PriceManager::PriceManager(QObject *parent) :
    PriceManager(nullptr, parent)
{
}

bool PriceManager::start()
{
    qCDebug(logPriceManager) << "Starting PriceManager";
    emit starting();
    m_running = true;
    runPriceRefresh();
    m_timer->start(DEFAULT_REFRESH_PERIOD_MILLISECONDS);
    return true;
}

bool PriceManager::stop()
{
    qCDebug(logPriceManager) << "Stopping PriceManager";
    emit stopping();
    m_timer->stop();
    m_running = false;
    emit stopped();
    return true;
}

void PriceManager::runPriceRefresh() const
{
    qCDebug(logPriceManager) << "Running PriceRefresh";
    if (refreshing()) {
        qCDebug(logPriceManager) << "Currently in process of refreshing; NOOP";
        return;
    }

    if (!m_currentPriceSource || !m_currentCurrency) {
        qDebug() << "Cannot refresh price without a price source or currency set; NOOP";
        return;
    }

    emit priceRefreshStarted();
    m_refreshing = true;

    qCDebug(logPriceManager) << "Constructing request";
    QUrl reqUrl = m_currentPriceSource->renderUrl(m_currentCurrency);
    qDebug() << "Constructed URL " << reqUrl.toDisplayString();

    QNetworkRequest request;
    request.setUrl(reqUrl);
    request.setRawHeader("User-Agent", "monero-gui/0.12.0 (Qt)");

    qDebug() << "Request constructed";

    QNetworkReply *reply = m_manager->get(request);

    qDebug() << "Request sent to manager, reply pointer responded";

    connect(reply, SIGNAL(finished()), this, SLOT(handleHTTPFinished()));
    connect(this, SIGNAL(stopping()), reply, SLOT(abort()));

    qDebug() << "Signals connected";
}

void PriceManager::handleHTTPFinished() const
{
    qDebug() << "Network reply has finished";
    m_refreshing = false;
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError)
        updatePrice(reply);
    else
        handleNetworkError(reply);

    reply->deleteLater();
}

void PriceManager::updatePrice(QNetworkReply* reply) const
{
    qDebug() << "Updating price";

    QByteArray data = reply->readAll();

    qDebug() << "Got response data " << QString::fromUtf8(data.data());

    QJsonParseError *error = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(data, error);

    qDebug() << "Parse QJsonDocument";

    if (doc.isNull()) {
        handleError(error->errorString());
    }

    if (!doc.isObject()) {
        handleError("Could not parse response JSON as object: " + doc.toJson());
        return;
    }

    bool success = m_currentPriceSource->updatePriceFromReply(m_currentPrice, m_currentCurrency, doc);
    /*
    QJsonObject respObj = doc.object();

    qDebug() << "got respObj";
    QJsonValue dataVal = respObj.value("data");

    qDebug() << "parsed data dataval";

    if (dataVal.isUndefined()) {
        handleError("Key 'data' not present in JSON response: " + doc.toJson());
        return;
    }

    if (!dataVal.isObject()) {
        handleError("Key 'data' in JSON response is not an object: " + doc.toJson());
        return;
    }

    QJsonValue quotesVal = dataVal.toObject().value("quotes");

    qDebug() << "parsed quotesVal";

    if (quotesVal.isUndefined()) {
        handleError("Key 'data.quotes' not present in JSON response: " + doc.toJson());
        return;
    }

    if (!quotesVal.isObject()) {
        handleError("Key 'data.quotes' in JSON response is not an object: " + doc.toJson());
        return;
    }

    QJsonValue currencyVal = quotesVal.toObject().value("USD");

    qDebug() << "parsed currencyVal";

    if (currencyVal.isUndefined()) {
        handleError("Key 'data.quotes.currency' not present in JSON response: " + doc.toJson());
        return;
    }

    if (!currencyVal.isObject()) {
        handleError("Key 'data.quotes.currency' in JSON response is not an object: " + doc.toJson());
        return;
    }

    QJsonValue priceVal = currencyVal.toObject().value("price");

    qDebug() << "parsed priceVal";

    if (priceVal.isUndefined()) {
        handleError("Key 'data.quotes.currency.price' not present in JSON response: " + doc.toJson());
        return;
    }

    if (!priceVal.isDouble()) {
        handleError("Key 'data.quotes.currency.price' in JSON response is not a double: " + doc.toJson());
        return;
    }

    qDebug() << "Parsed JSON, updating price";

    qreal price = static_cast<qreal>(priceVal.toDouble());

    qDebug() << "Static cast price";
    m_currentPrice->update(price, Currencies::USD);
    */
    qDebug() << "Got price " << m_currentPrice->price() << " for " << m_currentPrice->currency()->code();
    if (success)
        emit priceRefreshed();
}

void PriceManager::handleError(const QString &msg) const
{
    qDebug() << "Error: " << msg;
}

void PriceManager::handleNetworkError(const QNetworkReply* reply) const
{
    emit networkError();
    handleError("Network error: " + reply->errorString());
}

Price * PriceManager::price() const
{
    return m_currentPrice;
}

QString PriceManager::convert(quint64 amount) const
{
    return m_currentPrice->convert(amount);
}

QSet<PriceSource *> PriceManager::priceSourcesAvailable() const
{
    return m_priceSources;
}

QStringListModel *PriceManager::priceSourcesAvailableModel() const
{
    if (!m_priceSourcesAvailableModel) {
        PriceManager * pm = const_cast<PriceManager*>(this);
        QStringList l;
        for (const PriceSource * p : m_priceSources)
            l.append(p->label());
        m_priceSourcesAvailableModel = new QStringListModel(l, pm);
    }

    return m_priceSourcesAvailableModel;
}

CurrencySet PriceManager::currenciesAvailable() const
{
    if (!m_currentPriceSource)
        return CurrencySet();
    return m_currentPriceSource->currenciesAvailable();
}

bool PriceManager::updateCurrenciesAvailable()
{
    QStringList l;
    for (const Currency * c : currenciesAvailable())
        l.append(c->code());
    currenciesAvailableModel()->setStringList(l);
    return true;
}

QStringListModel *PriceManager::currenciesAvailableModel() const
{
    if (!m_currenciesAvailableModel) {
        PriceManager * pm = const_cast<PriceManager*>(this);
        m_currenciesAvailableModel = new QStringListModel(pm);
    }
    return m_currenciesAvailableModel;
}

bool PriceManager::setPriceSource(int index)
{
    /*
    QVariant p = m_priceSourcesAvailableModel->data(index);
    if (!p.isValid() || p.isNull()) {
        qDebug() << "Invalid price source index passed, NOOP";
        return;
    }

    if (!p.canConvert<PriceSource>()) {
        qDebug() << "Did not get a price source in the model -- this should never happen";
        return;
    }

    m_currentPriceSource = p.value<PriceSource>();
    updateCurrenciesAvailable();
    */
    return true;
}

PriceSource *PriceManager::currentPriceSource() const
{
    return m_currentPriceSource;
}

Currency * PriceManager::currentCurrency() const
{
    return m_currentCurrency;
}

bool PriceManager::priceAvailable() const
{
    return m_currentPrice->price() != 0;
}


bool PriceManager::running() const
{
    return m_running;
}

bool PriceManager::refreshing() const
{
    return m_refreshing;
}
