#ifndef PRICESOURCESELECTORMODEL_H
#define PRICESOURCESELECTORMODEL_H

#include "PriceSource.h"

#include <QAbstractListModel>
#include <QObject>

class PriceSourceSelectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PriceSourceSet availablePriceSources READ availablePriceSources)
public:
    enum PriceSourceViewRole {
        PriceSourceRole =  Qt::UserRole + 1,
        PriceSourceLabelRole,
        PriceSourceUrlRole,
        PriceSourceAvailableCurrenciesRole
    };
    Q_ENUM(PriceSourceViewRole)
    explicit PriceSourceSelectorModel(QObject * parent = nullptr, const PriceSourceSet * available = nullptr);
    const PriceSourceSet * availablePriceSources() const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QHash<int, QByteArray> roleNames() const  override;

private:
    friend class PriceManager;
    const PriceSourceSet * m_availablePriceSources;
};

#endif // PRICESOURCESELECTORMODEL_H
