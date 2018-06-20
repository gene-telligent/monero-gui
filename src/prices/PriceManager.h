#ifndef PRICEMANAGER_H
#define PRICEMANAGER_H

#include <QTimer>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QSet>
#include <QModelIndex>

#include "PriceSource.h"
#include "currency.h"

class Price;
class CurrencySelectorModel;
class PriceSourceSelectorModel;

class PriceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running)
    Q_PROPERTY(bool refreshing READ refreshing)
    Q_PROPERTY(bool priceAvailable READ priceAvailable NOTIFY priceRefreshed)
    Q_PROPERTY(Currency * currentCurrency READ currentCurrency)
    Q_PROPERTY(Price * price READ price)
    Q_PROPERTY(const PriceSourceSet priceSourcesAvailable READ priceSourcesAvailable)
    Q_PROPERTY(PriceSource * currentPriceSource READ currentPriceSource)

public:
    static PriceManager * instance(QNetworkAccessManager *manager);

    // Start the price polling thread
    Q_INVOKABLE void start();
    // Stop the price polling thread
    Q_INVOKABLE void stop();
    // Restart the price polling thread
    Q_INVOKABLE void restart();
    // Is there a price available yet?
    Q_INVOKABLE bool priceAvailable() const;
    // Are we running?
    Q_INVOKABLE bool running() const;
    // Are we refreshing the price?
    Q_INVOKABLE bool refreshing() const;
    // Get the current currency
    Q_INVOKABLE Currency * currentCurrency() const;
    // Get the current price
    Q_INVOKABLE Price *price() const;
    // Convert the amount given at the current price and currency
    Q_INVOKABLE QString convert(quint64 amount) const;
    // Get current price source
    Q_INVOKABLE PriceSource * currentPriceSource() const;
    // Set price source
    Q_INVOKABLE void setPriceSource(QModelIndex index);
    // Set currency
    Q_INVOKABLE void setCurrency(QModelIndex index);
    // Get price sources which are available
    Q_INVOKABLE PriceSourceSet priceSourcesAvailable() const;
    // Get the available price sources in a StringListModel for display
    Q_INVOKABLE PriceSourceSelectorModel *priceSourcesAvailableModel() const;
    // Get list of available currencies
    Q_INVOKABLE CurrencySet currenciesAvailable() const;
    // Get the available currencies in a StringListModel for display
    Q_INVOKABLE CurrencySelectorModel * currenciesAvailableModel() const;

    Q_INVOKABLE void handleError(const QString &msg) const;

private:
    void updatePrice(QNetworkReply *reply) const;
    void updateCurrenciesAvailable();

signals:
    void starting() const;
    void started() const;
    void priceRefreshStarted() const;
    void priceRefreshed() const;
    void priceSourceChanged() const;
    void currencyChanged() const;
    void networkError() const;
    void stopping() const;
    void stopped() const;

public slots:
    // Called when the timer hits (perform HTTP request)
    void runPriceRefresh() const;
    // Called when the HTTP request completes
    void handleHTTPFinished() const;
    // Called when the HTTP request fails
    void handleNetworkError(const QNetworkReply *reply) const;
    // Called when the PriceSource is changed
    void updateCurrenciesAvailable() const;

private:
    explicit PriceManager(QNetworkAccessManager *manager, QObject *parent = 0);
    explicit PriceManager(QObject *parent = 0);
    static PriceManager * m_instance;
    mutable bool m_running;
    mutable bool m_refreshing;
    QNetworkAccessManager * m_manager;
    QTimer * m_timer;
    Price * m_currentPrice;
    Currency * m_currentCurrency;
    PriceSource * m_currentPriceSource;
    const PriceSourceSet m_priceSources = {PriceSources::CoinMarketCap};
    mutable PriceSourceSelectorModel * m_priceSourcesAvailableModel;
    mutable CurrencySelectorModel * m_currenciesAvailableModel;

};

#endif // PRICEMANAGER_H
