#include "currency.h"

#include "QLocale"

/*
namespace DisplayCurrencies {
    const Currency USD = Currency(
                QString("USD"),
                QChar('$'),
                2);
    static const Currency GBP = Currency(
                QString("GBP"),
                QChar('£'),
                2);
    const Currency BTC = Currency(
                QString("BTC"),
                QChar(0x20BF),
                9);
}*/

Currency::Currency(const QString code, const QChar symbol, const int precision, QObject *parent) :
    QObject(parent),
    m_code(code),
    m_symbol(symbol),
    m_precision(precision)
{
}

QString Currency::code() const
{
    return m_code;
}

QChar Currency::symbol() const
{
    return m_symbol;
}

int Currency::precision() const
{
    return m_precision;
}

QString Currency::render(qreal amount) const
{
    return QLocale().toCurrencyString(amount, m_symbol, m_precision);
}



