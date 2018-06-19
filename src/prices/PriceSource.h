#ifndef PRICESOURCE_H
#define PRICESOURCE_H

#include "currency.h"

#include <QObject>
#include <QStringList>
#include <QSet>
#include <QUrl>
#include <QString>
#include <QNetworkReply>

class Price;

/* right now going to start with just coinmarketcap */
class PriceSource : public QObject
{
    Q_OBJECT

    // Label of the source, IE, "CoinMarketCap"
    Q_PROPERTY(QString label READ label)
    // URL used to contact the API for prices
    Q_PROPERTY(QUrl baseUrl READ baseUrl)
    // Supported currencies
    Q_PROPERTY(QStringList currencyCodes READ currencyCodes)
public:
    explicit PriceSource(QObject *parent = nullptr);
    QString label() const;
    QUrl baseUrl() const;
    QSet<Currency*> currencies() const;
    QStringList currencyCodes() const;
    Q_INVOKABLE Currency * currencyFor(QString code) const;

private:
    void updatePriceFromReply(Price * price, Currency * currency, QJsonDocument & reply);
    QUrl renderUrl(Currency * currency);

private:
    const QString m_label = QString("CoinMarketCap");
    const QUrl m_base_url = QUrl("https://api.coinmarketcap.com/v2/ticker/328/");
    const QSet<Currency*> m_currencies = {Currencies::USD, Currencies::GBP, Currencies::BTC};
    const QString m_json_path = "data/quotes/{CURRENCY}/price";
};

namespace PriceSources {
    extern PriceSource * const CoinMarketCap;
}

#endif // PRICESOURCE_H
