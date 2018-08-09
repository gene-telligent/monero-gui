#ifndef PRICESOURCE_H
#define PRICESOURCE_H

#include "currency.h"

#include <QObject>
#include <QUrl>
#include <QString>
#include <QList>

class Price;

class PriceSource : public QObject
{
    Q_OBJECT

    // Label of the source, IE, "CoinMarketCap"
    Q_PROPERTY(QString label READ label)
    // URL used to contact the API for prices
    Q_PROPERTY(QUrl baseUrl READ baseUrl)

public:
    explicit PriceSource(QObject *parent = nullptr) : QObject(parent) {}
    virtual QString label() const = 0;
    virtual QUrl baseUrl() const = 0;
    // Get list of available currencies
    // TODO: change this to a QVariantList for better compatibility with Javascript Data Binding
    Q_INVOKABLE virtual QList<Currency*> currenciesAvailable() const = 0;
    virtual bool updatePriceFromReply(Price * price, Currency * currency, QJsonDocument & reply) = 0;
    virtual QUrl renderUrl(Currency * currency) = 0;

private:
    friend class PriceManager;
};

class CoinMarketCapSource : public PriceSource
{
    Q_OBJECT

public:
    using PriceSource::PriceSource;
    QString label() const {return QLatin1Literal("CoinMarketCap"); }
    QUrl baseUrl() const {return m_baseUrl; }
    QList<Currency*> currenciesAvailable() const { return m_currenciesAvailable; }
    bool updatePriceFromReply(Price *price, Currency *currency, QJsonDocument &reply);
    QUrl renderUrl(Currency *currency);

private:
    const QList<Currency*> m_currenciesAvailable = {Currencies::USD, Currencies::BTC, Currencies::GBP};
    const QUrl m_baseUrl = QUrl(QLatin1Literal("https://api.coinmarketcap.com/v2/ticker/328/"));
    const QString m_jsonPath = QLatin1Literal("data/quotes/{CURRENCY}/price");
};

class BinanceSource : public PriceSource
{
    Q_OBJECT

public:
    using PriceSource::PriceSource;
    QString label() const {return QLatin1Literal("Binance"); }
    QUrl baseUrl() const {return m_baseUrl; }
    QList<Currency*> currenciesAvailable() const { return m_currenciesAvailable; }
    bool updatePriceFromReply(Price *price, Currency *currency, QJsonDocument &reply);
    QUrl renderUrl(Currency *currency);

private:
    const QList<Currency*> m_currenciesAvailable = {Currencies::BTC};
    const QUrl m_baseUrl = QUrl(QLatin1Literal("https://api.binance.com/api/v1/ticker/24hr"));
};

namespace PriceSources {
    extern PriceSource * const CoinMarketCap;
    extern PriceSource * const Binance;
}

#endif // PRICESOURCE_H
