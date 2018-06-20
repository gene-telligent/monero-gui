#include "PriceSourceSelectorModel.h"


PriceSourceSelectorModel::PriceSourceSelectorModel(QObject *parent, const PriceSourceSet *available) :
    QAbstractListModel(parent), m_availablePriceSources(available)
{

}

const PriceSourceSet *PriceSourceSelectorModel::availablePriceSources() const
{
    return m_availablePriceSources;
}

QVariant PriceSourceSelectorModel::data(const QModelIndex &index, int role) const
{
    if (!m_availablePriceSources) {
        return QVariant();
    }

    if (index.row() < 0 || (unsigned)index.row() >= m_availablePriceSources->count()) {
        return QVariant();
    }

    PriceSource * priceSource = m_availablePriceSources->at(index.row());
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
    return m_availablePriceSources ? m_availablePriceSources->count() : 0;
}

QHash<int, QByteArray> PriceSourceSelectorModel::roleNames() const
{
    static QHash<int, QByteArray> roleNames;
    if (roleNames.empty())
    {
        roleNames.insert(PriceSourceRole, "priceSource");
        roleNames.insert(PriceSourceLabelRole, "label");
        roleNames.insert(PriceSourceUrlRole, "baseUrl");
        roleNames.insert(PriceSourceAvailableCurrenciesRole, "availableCurrencies");
    }
    return roleNames;
}
