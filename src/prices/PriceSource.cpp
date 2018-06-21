#include "PriceSource.h"

#include "currency.h"
#include "Price.h"
#include "qtjsonpath.h"
#include "logging.h"
#include <QVariant>
#include <QDebug>
#include <QMetaType>
#include <QUrl>
#include <QUrlQuery>
#include <QStringListModel>

namespace PriceSources {
    PriceSource * const CoinMarketCap = new PriceSource(
                QString("CoinMarketCap"),
                QString("https://api.coinmarketcap.com/v2/ticker/328/"),
                {Currencies::USD, Currencies::GBP, Currencies::BTC},
                "data/quotes/{CURRENCY}/price");
    PriceSource * const Binance = new PriceSource(
                QString("Binance"),
                QString("https://api.binance.com/api/v1/ticker/24hr?symbol=XMR{CURRENCY}"),
                {Currencies::USD, Currencies::GBP, Currencies::BTC},
                "lastPrice");
}


PriceSource::PriceSource(QString label, QString baseUrl, QList<Currency*> supportedCurrencies, QString jsonPath, QObject *parent) :
    m_label(label),
    m_base_url(baseUrl),
    m_currencies(supportedCurrencies),
    m_json_path(jsonPath),
    QObject(parent)
{

}

QString PriceSource::label() const
{
    return m_label;
}

QString PriceSource::baseUrl() const
{
    return m_base_url;
}

QList<Currency*> PriceSource::currenciesAvailable() const
{
    qDebug() << "currencies available called from pricesource";
    qDebug() << "currencies are: " << m_currencies;
    return m_currencies;
}

QUrl PriceSource::renderUrl(Currency * currency)
{
    QString renderedBaseUrl(m_base_url);
    renderedBaseUrl.replace(QLatin1Literal("{CURRENCY}"), currency->label());
    QUrl renderedUrl(renderedBaseUrl);
    return renderedUrl;
}

bool PriceSource::updatePriceFromReply(Price *price, Currency * currency, QJsonDocument &reply)
{
    QtJsonPath walker(reply);
    QString modpath(m_json_path);
    modpath.replace(QLatin1Literal("{CURRENCY}"), currency->label());
    qDebug() << "Using path of " << modpath;
    QVariant res = walker.getValue(modpath);
    qDebug() << "Got walked value " << res << " with metatype " << res.userType();
    if (!res.isValid() || res.isNull() || res.userType() != QMetaType::Double) {
        qDebug() << "Invalid parsing of response from JSON reply; ignoring price update";
        return false;
    }
    QJsonValue val = res.toJsonValue();
    if (!val.isDouble()) {
        qDebug() << "Value at JsonPath was not numeric; ignoring price update";
        return false;
    }

    price->update(val.toDouble(), currency);
    return true;
}

