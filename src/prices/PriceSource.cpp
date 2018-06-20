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

PriceSource::PriceSource(QObject *parent) :
    QObject(parent)
{

}

QString PriceSource::label() const
{
    return m_label;
}

QUrl PriceSource::baseUrl() const
{
    return m_base_url;
}

CurrencySet PriceSource::currenciesAvailable() const
{
    return m_currencies;
}

QUrl PriceSource::renderUrl(Currency * currency)
{
    if (currency == Currencies::USD)
        return m_base_url;

    QUrl renderedUrl(m_base_url);
    QUrlQuery query = QUrlQuery();
    query.addQueryItem(QStringLiteral("convert"), currency->label());
    renderedUrl.setQuery(query);
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

namespace PriceSources {
    PriceSource * const CoinMarketCap = new PriceSource();
}
