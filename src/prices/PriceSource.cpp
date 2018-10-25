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

namespace PriceSources {
    PriceSource * const CoinMarketCap = new CoinMarketCapSource();
    PriceSource * const Binance = new BinanceSource();
}

QUrl CoinMarketCapSource::renderUrl(Currency * currency)
{
    QUrl rendered(m_baseUrl);
    QUrlQuery query = QUrlQuery();
    query.addQueryItem(QLatin1Literal("convert"), currency->label());
    rendered.setQuery(query);
    return rendered;
}

bool CoinMarketCapSource::updatePriceFromReply(Price *price, Currency * currency, QJsonDocument &reply)
{
    QtJsonPath walker(reply);
    QString modpath = QString(QLatin1Literal("data/quotes/"))
            .append(currency->label())
            .append(QLatin1Literal("/price"));
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

QUrl BinanceSource::renderUrl(Currency * currency)
{
    QUrl rendered(m_baseUrl);
    QUrlQuery query = QUrlQuery();
    QString pair(QLatin1Literal("XMR"));
    pair.append(currency->label());
    query.addQueryItem(QLatin1Literal("symbol"), pair);
    rendered.setQuery(query);
    return rendered;
}

bool BinanceSource::updatePriceFromReply(Price *price, Currency * currency, QJsonDocument &reply)
{
    qDebug() << "got reply" << reply.toJson();

    if (!reply.isObject() || reply.isEmpty()) {
        qDebug() << "Invalid JSON response [reply is not object or empty]; ignoring price update";
        return false;
    }

    QString priceRaw = reply
            .object()
            .value(QLatin1Literal("lastPrice"))
            .toString();
    if (priceRaw.isNull()) {
        qDebug() << "Invalid JSON response [does not contain valid key 'lastPrice']; ignoring price update";
        return false;
    }

    bool success;
    qreal priceDouble = priceRaw.toDouble(&success);
    if (!success) {
        qDebug() << "Invalid JSON response [value " << priceRaw << " at key 'lastPrice' cannot be converted to double]; ignoring price update";
        return false;
    }

    price->update(priceDouble, currency);
    return true;
}


