#ifndef CURRENCY_H
#define CURRENCY_H

#include <QObject>
#include <QSet>

class Currency : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code READ code)
    Q_PROPERTY(QChar symbol READ symbol)
    Q_PROPERTY(int precision READ precision)
public:
    explicit Currency(const QString code = 0, const QChar symbol = 0, const int precision = 0, QObject *parent = nullptr);

    QString code() const;
    QChar symbol() const;
    int precision() const;
    Q_INVOKABLE QString render(qreal amount) const;

private:
    const QString m_code;
    const QChar m_symbol;
    const int m_precision;
};

namespace Currencies {
    extern Currency * const USD;
    extern Currency * const GBP;
    extern Currency * const BTC;
}

typedef QSet<Currency *> CurrencySet;
#endif // CURRENCY_H
