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
    if (index.row() < 0 || (unsigned)index.row() >= m_availableCurrencies.count())
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
    }
    return roleNames;
}

void CurrencySelectorModel::setAvailableCurrencies(QList<Currency*> currencies)
{
    qDebug() << "beginning to reset model";
    qDebug() << "Got currency list " << currencies;
    beginResetModel();
    qDebug() << "assigning available currencies";
    m_availableCurrencies = QList<Currency*>(currencies);
    qDebug() << "available currencies assigned";
    endResetModel();
    qDebug() << "Reset model emitted, emitting availablecurrencieschanged";
    emit availableCurrenciesChanged();
    qDebug() << "available currencies changed emitted";
}
