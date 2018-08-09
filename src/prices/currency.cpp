#include "currency.h"

#include "QLocale"

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
                // Using a Thai Bhat symbol right now since most OS fonts still don't support Unicode 10
                QChar(0x0E3F),
                9);
}


Currency::Currency(const QString label, const QChar symbol, const int precision, QObject *parent) :
    QObject(parent),
    m_label(label),
    m_symbol(symbol),
    m_precision(precision)
{
}

QString Currency::label() const
{
    return m_label;
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






