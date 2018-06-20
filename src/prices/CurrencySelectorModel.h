#ifndef CURRENCYSELECTORMODEL_H
#define CURRENCYSELECTORMODEL_H

#include <QAbstractListModel>
#include "currency.h"

class CurrencySelectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(CurrencySet availableCurrencies READ availableCurrencies NOTIFY availableCurrenciesChanged)
public:
    enum CurrencyViewRole {
        CurrencyRole =  Qt::UserRole + 1,
        CurrencyLabelRole,
        CurrencySymbolRole
    };
    Q_ENUM(CurrencyViewRole)

    explicit CurrencySelectorModel(QObject *parent = nullptr);
    CurrencySet * availableCurrencies() const;


    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QHash<int, QByteArray> roleNames() const  override;


private:
    void setAvailableCurrencies(const CurrencySet currencies);
signals:
    void availableCurrenciesChanged();

public slots:

private:
    friend class PriceManager;
    CurrencySet * m_availableCurrencies;
};

#endif // CURRENCYSELECTORMODEL_H
