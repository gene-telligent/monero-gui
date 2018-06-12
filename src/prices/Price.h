#ifndef PRICE_H
#define PRICE_H

#include "currency.h"
#include <QObject>
#include <QDateTime>
#include <QString>

class PriceManager;

class Price : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal price READ price NOTIFY updated)
    Q_PROPERTY(QString currencyCode READ currencyCode NOTIFY updated)
    Q_PROPERTY(QDateTime lastUpdated READ lastUpdated NOTIFY updated)
    Q_PROPERTY(bool stale READ stale NOTIFY updated)
public:
    Q_INVOKABLE qreal price() const;
    Q_INVOKABLE QString currencyCode() const;
    Q_INVOKABLE QDateTime lastUpdated() const;
    Q_INVOKABLE bool stale() const;
    Q_INVOKABLE QString convert(quint64 amount) const;

signals:
    // Emitted when update is called
    void updated();

private:
    explicit Price(QObject *parent = nullptr);
    void update(qreal price, Currency  * currency);

private:
    friend class PriceManager;
    friend class PriceSource;
    qreal m_price;
    Currency * m_currency;
    QDateTime m_lastUpdated;
};

#endif // PRICE_H
