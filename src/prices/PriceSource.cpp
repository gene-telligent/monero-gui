#include "PriceSource.h"

#include "currency.h"
#include "qtjsonpath.h"


PriceSource::PriceSource(const QString label, const QUrl url, const QSet<Currency *> currencies, const QString path, QObject *parent) :
    QObject(parent),
    m_label(label),
    m_url(url),
    m_currencies(currencies),
    m_json_path(path)
{

}

Currency *PriceSource::currency(const QString code) const
{
    for (QSet<Currency *>::const_iterator iter = m_currencies.constBegin(); iter != m_currencies.constEnd(); iter++) {
        if ((*iter)->code() == code)
            return (*iter);
    }
    return nullptr;
}

QSet<Currency *> PriceSource::currencies() const
{
    return m_currencies;
}

QString PriceSource::label() const
{
    return m_label;
}

QUrl PriceSource::url() const
{
    return m_url;
}

QUrl PriceSource::generatedUrl() const
{

}

void PriceSource::updatePriceFromReply(Price *price, QJsonDocument &reply)
{
    QtJsonPath walker(reply);

}
