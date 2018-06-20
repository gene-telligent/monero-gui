#include "PriceManager.h"
#include "Price.h"
#include "currency.h"
#include "prices/logging.h"
#include "PriceSource.h"
#include "CurrencySelectorModel.h"
#include "PriceSourceSelectorModel.h"

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
    connect(this, SIGNAL(priceSourceChanged()), this, SLOT(updateCurrenciesAvailable()));
    connect(this, SIGNAL(currencyChanged()), this, SLOT(runPriceRefresh()));

    // DEBUGGING TIME
    m_currentPriceSource = PriceSources::CoinMarketCap;
    m_currentCurrency = Currencies::USD;

    start();
}

PriceManager::PriceManager(QObject *parent) :
    PriceManager(nullptr, parent)
{
}

void PriceManager::start()
{
    qCDebug(logPriceManager) << "Starting PriceManager";
    emit starting();
    m_running = true;
    runPriceRefresh();
    m_timer->start(DEFAULT_REFRESH_PERIOD_MILLISECONDS);
}

void PriceManager::stop()
{
    qCDebug(logPriceManager) << "Stopping PriceManager";
    emit stopping();
    m_timer->stop();
    m_running = false;
    emit stopped();
}

void PriceManager::restart()
{
    stop();
    start();
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

    qDebug() << "Got price " << m_currentPrice->price() << " for " << m_currentPrice->currency()->label();
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

PriceSourceSet PriceManager::priceSourcesAvailable() const
{
    return m_priceSources;
}

PriceSourceSelectorModel *PriceManager::priceSourcesAvailableModel() const
{
    if (!m_priceSourcesAvailableModel) {
        PriceManager * pm = const_cast<PriceManager*>(this);
        //PriceSourceSet * pss = const_cast<PriceSourceSet*>(m_priceSources);
        m_priceSourcesAvailableModel = new PriceSourceSelectorModel(pm, &m_priceSources);
    }

    return m_priceSourcesAvailableModel;
}

CurrencySet PriceManager::currenciesAvailable() const
{
    if (!m_currentPriceSource)
        return CurrencySet();
    return m_currentPriceSource->currenciesAvailable();
}

void PriceManager::updateCurrenciesAvailable()
{
    m_currenciesAvailableModel->setAvailableCurrencies(currenciesAvailable());
}

CurrencySelectorModel *PriceManager::currenciesAvailableModel() const
{
    if (!m_currenciesAvailableModel) {
        PriceManager * pm = const_cast<PriceManager*>(this);
        m_currenciesAvailableModel = new CurrencySelectorModel(pm);
    }
    return m_currenciesAvailableModel;
}

void PriceManager::setPriceSource(QModelIndex index)
{
    QVariant p = m_priceSourcesAvailableModel->data(index, PriceSourceSelectorModel::PriceSourceRole);
    if (!p.isValid() || p.isNull()) {
        qDebug() << "Invalid price source index passed, NOOP";
        return;
    }

    if (!p.canConvert<PriceSource *>()) {
        qDebug() << "Did not get a price source in the model -- this should never happen";
        return;
    }

    m_currentPriceSource = p.value<PriceSource *>();
    emit priceSourceChanged();
}

void PriceManager::setCurrency(QModelIndex index)
{
    QVariant c = m_currenciesAvailableModel->data(index, CurrencySelectorModel::CurrencyRole);
    if (!c.isValid() || c.isNull()) {
        qDebug() << "Invalid currency index passed, NOOP";
        return;
    }

    if (!c.canConvert<Currency *>()) {
        qDebug() << "Did not get a price source in the model -- this should never happen";
        return;
    }

    m_currentCurrency = c.value<Currency *>();
    emit currencyChanged();
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
