#include "currency.h"

#include "QLocale"

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


namespace Currencies {
    Currency * const USD = new Currency(
                QString("USD"),
                QChar('$'),
                2);
    Currency * const GBP = new Currency(
                QString("GBP"),
                QChar(0x00A3),
                2);
    Currency * const BTC = new Currency(
                QString("BTC"),
                QChar(0x20BF),
                9);
}




