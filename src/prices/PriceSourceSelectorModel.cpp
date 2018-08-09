#include "PriceSourceSelectorModel.h"
#include <QDebug>


PriceSourceSelectorModel::PriceSourceSelectorModel(QObject *parent, QList<PriceSource*> available) :
    QAbstractListModel(parent), m_availablePriceSources(available)
{
}

QList<PriceSource*> PriceSourceSelectorModel::availablePriceSources() const
{
    return m_availablePriceSources;
}

QVariant PriceSourceSelectorModel::data(const QModelIndex &index, int role) const
{
    if (m_availablePriceSources.empty()) {
        qDebug() << "No available price sources configured!";
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= m_availablePriceSources.count()) {
        qDebug() << "Index OOB for price source selection";
        return QVariant();
    }

    PriceSource * priceSource = m_availablePriceSources.at(index.row());
    Q_ASSERT(priceSource);
    if (!priceSource) {
        qCritical("%s: internal error: no priceSource info for index %d", __FUNCTION__, index.row());
        return QVariant();
    }

    QVariant result;
    switch (role) {
    case PriceSourceRole:
        result = QVariant::fromValue(priceSource);
        break;
    case PriceSourceLabelRole:
    case PriceSourceSimpleDropdownRole:
        result = priceSource->label();
        break;
    case PriceSourceUrlRole:
        result = priceSource->baseUrl();
        break;
    case PriceSourceAvailableCurrenciesRole:
        result = QVariant::fromValue(priceSource->currenciesAvailable());
        break;
    }
    return result;
}

int PriceSourceSelectorModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_availablePriceSources.count();
}

QHash<int, QByteArray> PriceSourceSelectorModel::roleNames() const
{
    static QHash<int, QByteArray> roleNames;
    if (roleNames.empty())
    {
        roleNames.insert(PriceSourceRole, "priceSource");
        roleNames.insert(PriceSourceLabelRole, "label");
        roleNames.insert(PriceSourceSimpleDropdownRole, "column1");
        roleNames.insert(PriceSourceUrlRole, "baseUrl");
        roleNames.insert(PriceSourceAvailableCurrenciesRole, "availableCurrencies");
    }
    return roleNames;
}
