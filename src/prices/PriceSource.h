#ifndef PRICESOURCE_H
#define PRICESOURCE_H

#include <QObject>
#include <QStringList>
#include <QSet>
#include <QUrl>
#include <QString>
#include <QNetworkReply>

class Currency;
class Price;

class PriceSource : public QObject
{
    Q_OBJECT

    // Label of the source, IE, "CoinMarketCap"
    Q_PROPERTY(QString label READ label)
    // URL used to contact the API for prices
    Q_PROPERTY(QUrl url READ url)
    // Supported currencies
    //Q_PROPERTY(QStringList currencyCodes READ currencyCodes)

    Q_PROPERTY(QSet<Currency*> currencies READ currencies)
public:
    explicit PriceSource(const QString label = 0,
                         const QUrl url = QUrl(),
                         const QSet<Currency*> currencies = {},
                         const QString path = 0,
                         QObject *parent = nullptr);
    Q_INVOKABLE Currency* currency(const QString code) const;
    QString label() const;
    QUrl url() const;
    //QStringList currencyCodes() const;
    QSet<Currency*> currencies() const;

private:
    void updatePriceFromReply(Price * price, QJsonDocument & reply);

private:
    const QString m_label;
    const QUrl m_url;
    const QSet<Currency*> m_currencies;
    const QString m_json_path;
};

#endif // PRICESOURCE_H
