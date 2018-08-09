#include "CurrencySelectorModel.h"
#include <QDebug>

CurrencySelectorModel::CurrencySelectorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_availableCurrencies = QList<Currency*>();
}

QList<Currency*> CurrencySelectorModel::availableCurrencies() const
{
    return m_availableCurrencies;
}

QVariant CurrencySelectorModel::data(const QModelIndex &index, int role) const
{
    if (m_availableCurrencies.empty())
        return QVariant();
    if (index.row() < 0 || index.row() >= m_availableCurrencies.count())
        return QVariant();

    Currency * currency = m_availableCurrencies.at(index.row());
    Q_ASSERT(currency);
    if (!currency) {
        qCritical("%s: internal error: no currency info for index %d", __FUNCTION__, index.row());
        return QVariant();
    }

    QVariant result;
    switch (role) {
    case CurrencyRole:
        result = QVariant::fromValue(currency);
        break;
    case CurrencyLabelRole:
    case CurrencySimpleDropdownRole:
        result = currency->label();
        break;
    case CurrencySymbolRole:
        result = currency->symbol();
        break;
    }

    return result;
}

int CurrencySelectorModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_availableCurrencies.count();
}

QHash<int, QByteArray> CurrencySelectorModel::roleNames() const
{
    static QHash<int, QByteArray> roleNames;
    if (roleNames.empty())
    {
        roleNames.insert(CurrencyRole, "currency");
        roleNames.insert(CurrencyLabelRole, "label");
        roleNames.insert(CurrencySymbolRole, "symbol");
        roleNames.insert(CurrencySimpleDropdownRole, "column1");
    }
    return roleNames;
}

void CurrencySelectorModel::setAvailableCurrencies(QList<Currency*> currencies)
{
    beginResetModel();
    m_availableCurrencies = QList<Currency*>(currencies);
    endResetModel();
    emit availableCurrenciesChanged();
}
