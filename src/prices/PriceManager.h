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
    Q_PROPERTY(bool priceReady READ priceReady NOTIFY priceRefreshed)
    Q_PROPERTY(Currency * currentCurrency READ currentCurrency NOTIFY currencyChanged)
    Q_PROPERTY(QList<Currency*> currenciesAvailable READ currenciesAvailable NOTIFY priceSourceChanged)
    Q_PROPERTY(CurrencySelectorModel * currenciesAvailableModel READ currenciesAvailableModel NOTIFY priceSourceChanged)
    Q_PROPERTY(Price * price READ price NOTIFY priceRefreshed)
    Q_PROPERTY(const QList<PriceSource*> priceSourcesAvailable READ priceSourcesAvailable)
    Q_PROPERTY(PriceSourceSelectorModel * priceSourcesAvailableModel READ priceSourcesAvailableModel)
    Q_PROPERTY(PriceSource * currentPriceSource READ currentPriceSource NOTIFY priceSourceChanged)

public:
    static PriceManager * instance(QNetworkAccessManager *manager);

    // Start the price polling thread
    Q_INVOKABLE void start();
    // Stop the price polling thread
    Q_INVOKABLE void stop();
    // Restart the price polling thread
    //Q_INVOKABLE void restart();
    // Is there a price available yet?
    Q_INVOKABLE bool priceReady() const;
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
    void setPriceSource(int index);
    Q_INVOKABLE void setPriceSource(QModelIndex index);
    // Set currency
    void setCurrency(int index);
    Q_INVOKABLE void setCurrency(QModelIndex index);
    // Get price sources which are available
    QList<PriceSource*> priceSourcesAvailable() const;
    // Get the available price sources in a StringListModel for display
    PriceSourceSelectorModel *priceSourcesAvailableModel() const;
    // Get list of available currencies
    QList<Currency*> currenciesAvailable() const;
    // Get the available currencies in a StringListModel for display
    CurrencySelectorModel * currenciesAvailableModel() const;

    Q_INVOKABLE void handleError(const QString &msg) const;

private:
    void updatePrice(QNetworkReply *reply) const;

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
    Q_INVOKABLE void restart();
    // Called when the timer hits (perform HTTP request)
    void runPriceRefresh() const;
    // Called when the HTTP request completes
    void handleHTTPFinished() const;
    // Called when the HTTP request fails
    void handleNetworkError(const QNetworkReply *reply) const;
    // Called when the PriceSource is changed
    void updateCurrenciesAvailable();

private:
    explicit PriceManager(QNetworkAccessManager *manager, QObject *parent = nullptr);
    explicit PriceManager(QObject *parent = nullptr);
    static PriceManager * m_instance;
    mutable bool m_running;
    mutable bool m_refreshing;
    QNetworkAccessManager * m_manager;
    mutable QNetworkReply * m_reply;
    QTimer * m_timer;
    Price * m_currentPrice;
    Currency * m_currentCurrency;
    PriceSource * m_currentPriceSource;
    const QList<PriceSource*> m_priceSourcesAvailable = {PriceSources::CoinMarketCap, PriceSources::Binance};
    PriceSourceSelectorModel * m_priceSourcesAvailableModel;
    CurrencySelectorModel * m_currenciesAvailableModel;

};

#endif // PRICEMANAGER_H
