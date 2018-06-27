#ifndef CURRENCYSELECTORMODEL_H
#define CURRENCYSELECTORMODEL_H

#include <QAbstractListModel>
#include "currency.h"

class CurrencySelectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QList<Currency*> availableCurrencies READ availableCurrencies NOTIFY availableCurrenciesChanged)
public:
    enum CurrencyViewRole {
        CurrencyRole =  Qt::UserRole + 1,
        CurrencyLabelRole,
        CurrencySimpleDropdownRole,
        CurrencySymbolRole
    };
    Q_ENUM(CurrencyViewRole)

    explicit CurrencySelectorModel(QObject *parent = nullptr);
    QList<Currency*> availableCurrencies() const;


    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QHash<int, QByteArray> roleNames() const  override;


private:
    void setAvailableCurrencies(QList<Currency*> currencies);
signals:
    void availableCurrenciesChanged();

public slots:

private:
    friend class PriceManager;
    QList<Currency*> m_availableCurrencies;
};

#endif // CURRENCYSELECTORMODEL_H
