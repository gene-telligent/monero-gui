#include "PriceSource.h"

#include "currency.h"
#include "Price.h"
#include "qtjsonpath.h"
#include <QVariant>
#include <QDebug>
#include <QMetaType>
#include <QUrl>
#include <QUrlQuery>

PriceSource::PriceSource(QObject *parent) :
    QObject(parent)
{

}

QSet<Currency *> PriceSource::currencies() const
{
    return m_currencies;
}

QStringList PriceSource::currencyCodes() const
{
    for (const Currency * i : m_currencies) {

    }
}

QString PriceSource::label() const
{
    return m_label;
}

QUrl PriceSource::baseUrl() const
{
    return m_base_url;
}

QUrl PriceSource::renderUrl(Currency * currency)
{
    if (currency == Currencies::USD)
        return m_base_url;

    QUrl renderedUrl(m_base_url);
    QUrlQuery query = QUrlQuery();
    query.addQueryItem(QStringLiteral("convert"), currency->code());
    renderedUrl.setQuery(query);
    return renderedUrl;
}

void PriceSource::updatePriceFromReply(Price *price, Currency * currency, QJsonDocument &reply)
{
    QtJsonPath walker(reply);
    QString modpath(m_json_path);
    modpath.replace(QLatin1Literal("{CURRENCY}"), currency->code());
    QVariant res = walker.getValue(modpath);
    if (!res.isValid() || res.isNull() || res.userType() != QMetaType::QJsonValue) {
        qDebug() << "Invalid parsing of response from JSON reply; ignoring price update";
        return;
    }
    QJsonValue val = res.toJsonValue();
    if (!val.isDouble()) {
        qDebug() << "Value at JsonPath was not numeric; ignoring price update";
        return;
    }

    price->update(val.toDouble(), currency);
}

namespace PriceSources {
    PriceSource * const CoinMarketCap = new PriceSource();
}
