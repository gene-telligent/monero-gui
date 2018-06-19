#include "PriceManager.h"
#include "Price.h"
#include "currency.h"
#include "prices/logging.h"

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
    static const QString CURRENCY = "USD";
    static const QUrl URL = QUrl("https://api.coinmarketcap.com/v2/ticker/328/");
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
    m_price(nullptr),
    m_timer(nullptr)
{
    qDebug() << "instantiating pricemanager debug";
    qCDebug(logPriceManager) << "Instantiating PriceManager";
    m_running = false;
    m_refreshing = false;
    m_price = new Price(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(runPriceRefresh()));

    //start();
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

    emit priceRefreshStarted();
    m_refreshing = true;

    qCDebug(logPriceManager) << "Constructing request";

    QNetworkRequest request;
    request.setUrl(URL);
    request.setRawHeader("User-Agent", "monero-gui/0.12.0 (Qt)");

    qDebug() << "Requeest constructed";

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

    // TODO: fix this whole parsing section

    if (doc.isNull()) {
        handleError(error->errorString());
    }

    if (!doc.isObject()) {
        handleError("Could not parse response JSON as object: " + doc.toJson());
        return;
    }

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
    m_price->update(price, Currencies::USD);
    qDebug() << "Got price " << price << " for USD";
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
    return m_price;
}

QSet<PriceSource *> PriceManager::priceSourcesAvailable() const
{
    return m_price_sources;
}

PriceSource *PriceManager::currentPriceSource() const
{
    return m_currentPriceSource;
}

Currency * PriceManager::currency() const
{
    return m_price->currency();
}

bool PriceManager::priceAvailable() const
{
    return (m_price->price() != 0 && !m_price->stale());
}


bool PriceManager::running() const
{
    return m_running;
}

bool PriceManager::refreshing() const
{
    return m_refreshing;
}
