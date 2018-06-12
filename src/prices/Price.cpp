#include "Price.h"
#include "currency.h"

#include <QDateTime>
#include <QLocale>
#include <QDebug>

namespace {
    static const qint64 DEFAULT_STALE_TIME_MILLISECONDS = 900 * 1000;
    static const qint64 MONERO_STANDARD_UNIT = 1000000000000;
}

Price::Price(QObject *parent) : QObject(parent),
    m_price(0),
    m_currency(nullptr)
{
    m_lastUpdated = QDateTime();
}

void Price::update(qreal price, Currency *currency)
{
    m_price = price;
    m_currency = currency;
    m_lastUpdated = QDateTime::currentDateTimeUtc();
    qDebug() << "Updated price: " << m_price << ", currency: " << m_currency << ", last updated: " << m_lastUpdated.toString();
    emit updated();
}

QString Price::currencyCode() const
{
    return m_currency->code();
}

qreal Price::price() const
{
    return m_price;
}

QDateTime Price::lastUpdated() const
{
    return m_lastUpdated;
}

bool Price::stale() const
{
    return m_lastUpdated.isNull() ||
            (QDateTime::currentMSecsSinceEpoch() - m_lastUpdated.toMSecsSinceEpoch() > DEFAULT_STALE_TIME_MILLISECONDS);
}

QString Price::convert(quint64 amount) const
{
    qDebug() << "Got amount " << amount;

    if (stale()) {
        qDebug() << "Stale, returning null string";
        return QString();
    }

    // TODO: this almost certainly is an unsafe cast at high numerics
    qreal total = amount * m_price / MONERO_STANDARD_UNIT;
    //return QString::number(total, 'f', 2);
    return m_currency->render(total);
}
