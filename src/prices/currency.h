#ifndef CURRENCY_H
#define CURRENCY_H

#include <QObject>
#include <QList>

class Currency : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label)
    Q_PROPERTY(QChar symbol READ symbol)
    Q_PROPERTY(int precision READ precision)
public:
    explicit Currency(const QString label = nullptr, const QChar symbol = 0, const int precision = 0, QObject *parent = nullptr);

    QString label() const;
    QChar symbol() const;
    int precision() const;
    Q_INVOKABLE QString render(qreal amount) const;

private:
    const QString m_label;
    const QChar m_symbol;
    const int m_precision;
};

namespace Currencies {
    extern Currency * const USD;
    extern Currency * const GBP;
    extern Currency * const BTC;
}

typedef QList<Currency*> CurrencySet;
#endif // CURRENCY_H
