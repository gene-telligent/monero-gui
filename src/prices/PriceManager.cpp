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
    m_reply(nullptr),
    m_timer(nullptr),
    m_currentPrice(nullptr),
    m_currentCurrency(nullptr),
    m_currentPriceSource(nullptr),
    m_priceSourcesAvailableModel(nullptr),
    m_currenciesAvailableModel(nullptr)
{
    qDebug() << "instantiating pricemanager debug";
    qCDebug(logPriceManager) << "Instantiating PriceManager";
    m_running = false;
    m_refreshing = false;
    m_currentPrice = new Price(this);
    m_timer = new QTimer(this);

    PriceManager * pm = const_cast<PriceManager*>(this);
    m_priceSourcesAvailableModel = new PriceSourceSelectorModel(pm, m_priceSourcesAvailable);
    m_currenciesAvailableModel = new CurrencySelectorModel(pm);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(runPriceRefresh()));
    connect(this, SIGNAL(priceSourceChanged()), this, SLOT(updateCurrenciesAvailable()));
    connect(this, SIGNAL(currencyChanged()), this, SLOT(runPriceRefresh()));

    // DEBUGGING TIME
    //m_currentPriceSource = PriceSources::CoinMarketCap;
    //m_currentCurrency = Currencies::USD;

    //start();
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
    if (m_reply && m_reply->isRunning()) {
        m_reply->abort();
        m_reply->deleteLater();
    }
    m_running = false;
    emit stopped();
}

void PriceManager::restart()
{
    stop();
    start();
}

void PriceManager::runPriceRefresh()
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

    QUrl reqUrl = m_currentPriceSource->renderUrl(m_currentCurrency);

    QNetworkRequest request;
    request.setUrl(reqUrl);
    request.setRawHeader("User-Agent", "monero-gui/0.12.0 (Qt)");

    m_reply = m_manager->get(request);

    qDebug() << "Request sent to manager, reply pointer responded";

    connect(m_reply, SIGNAL(finished()), this, SLOT(handleHTTPFinished()));
    connect(this, SIGNAL(stopping()), m_reply, SLOT(abort()));

    qDebug() << "Signals connected";
}

void PriceManager::handleHTTPFinished()
{
    m_refreshing = false;

    Q_ASSERT(m_reply);
    if (!m_reply) {
        qDebug() << "HTTP Finish called but no m_reply stored.";
        return;
    }

    if (m_reply->error() == QNetworkReply::NoError)
        updatePrice();
    else
        handleNetworkError();

    m_reply->deleteLater();
}

void PriceManager::updatePrice()
{
    qDebug() << "Updating price";

    QByteArray data = m_reply->readAll();
    QJsonParseError *error = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(data, error);

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

void PriceManager::handleNetworkError()
{
    emit networkError();
    handleError("Network error: " + m_reply->errorString());
}

Price * PriceManager::price() const
{
    return m_currentPrice;
}

QString PriceManager::convert(quint64 amount) const
{
    return m_currentPrice->convert(amount);
}

QList<PriceSource*> PriceManager::priceSourcesAvailable() const
{
    return m_priceSourcesAvailable;
}


PriceSourceSelectorModel *PriceManager::priceSourcesAvailableModel() const
{
    return m_priceSourcesAvailableModel;
}


QList<Currency*> PriceManager::currenciesAvailable() const
{
    if (!m_currentPriceSource)
        return QList<Currency*>();
    return m_currentPriceSource->currenciesAvailable();
}

void PriceManager::setPriceSource(int index)
{
    if (index < 0 || index >= m_priceSourcesAvailable.count()) {
        qDebug() << "Bad index passed to setPriceSource; NOOP";
        return;
    }

    m_currentPriceSource = m_priceSourcesAvailable.at(index);
    emit priceSourceChanged();
}

void PriceManager::setCurrency(int index)
{
    if (index < 0 || index >= currenciesAvailable().count()) {
        qDebug() << "Bad index passed to setCurrency; NOOP";
        return;
    }

    m_currentCurrency = currenciesAvailable().at(index);
    emit currencyChanged();
}


void PriceManager::updateCurrenciesAvailable()
{
    this->currenciesAvailableModel()->setAvailableCurrencies(this->currenciesAvailable());
}


CurrencySelectorModel *PriceManager::currenciesAvailableModel() const
{
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

    qDebug() << "received variant " << p;
    m_currentPriceSource = p.value<PriceSource *>();
    qDebug() << "set current priceSource to " << m_currentPriceSource->label();
    emit priceSourceChanged();
    qDebug() << "change emitted";
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

bool PriceManager::priceReady() const
{
    if (m_currentPrice->currency() && !m_currentPrice->stale())
        return true;
    return false;
}


bool PriceManager::running() const
{
    return m_running;
}

bool PriceManager::refreshing() const
{
    return m_refreshing;
}
