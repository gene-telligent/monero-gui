#ifndef PRICESOURCE_H
#define PRICESOURCE_H

#include "currency.h"

#include <QObject>
#include <QUrl>
#include <QString>
#include <QList>

class Price;

/* right now going to start with just coinmarketcap */
class PriceSource : public QObject
{
    Q_OBJECT

    // Label of the source, IE, "CoinMarketCap"
    Q_PROPERTY(QString label READ label)
    // URL used to contact the API for prices
    Q_PROPERTY(QUrl baseUrl READ baseUrl)
public:
    explicit PriceSource(QString label = QString(), QString baseUrl = QString(), QList<Currency*> supportedCurrencies = {}, QString jsonPath = QString(), QObject *parent = nullptr);
    QString label() const;
    QString baseUrl() const;
    // Get list of available currencies
    Q_INVOKABLE QList<Currency*> currenciesAvailable() const;
private:
    bool updatePriceFromReply(Price * price, Currency * currency, QJsonDocument & reply);
    QUrl renderUrl(Currency * currency);

private:
    friend class PriceManager;
    const QString m_label;
    const QString m_base_url;
    const QList<Currency*> m_currencies;
    const QString m_json_path;
};

namespace PriceSources {
    extern PriceSource * const CoinMarketCap;
    extern PriceSource * const Binance;
}

//typedef QList<PriceSource *> PriceSourceSet;

#endif // PRICESOURCE_H
