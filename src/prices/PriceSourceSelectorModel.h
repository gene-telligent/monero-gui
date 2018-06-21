#ifndef PRICESOURCESELECTORMODEL_H
#define PRICESOURCESELECTORMODEL_H

#include "PriceSource.h"

#include <QAbstractListModel>
#include <QObject>

class PriceSourceSelectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QList<PriceSource*> availablePriceSources READ availablePriceSources NOTIFY availablePriceSourcesChanged)
public:
    enum PriceSourceViewRole {
        PriceSourceRole =  Qt::UserRole + 1,
        PriceSourceLabelRole,
        PriceSourceSimpleDropdownRole,
        PriceSourceUrlRole,
        PriceSourceAvailableCurrenciesRole
    };
    Q_ENUM(PriceSourceViewRole)
    explicit PriceSourceSelectorModel(QObject * parent = nullptr, QList<PriceSource*> available = QList<PriceSource*>());
    QList<PriceSource*> availablePriceSources() const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QHash<int, QByteArray> roleNames() const  override;

signals:
    void availablePriceSourcesChanged() const;

private:
    friend class PriceManager;
    QList<PriceSource*> m_availablePriceSources;
};

#endif // PRICESOURCESELECTORMODEL_H
