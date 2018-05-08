#ifndef PRICEMANAGER_H
#define PRICEMANAGER_H

#include <QTimer>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class Price;

class PriceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running)
    Q_PROPERTY(bool refreshing READ refreshing)
    Q_PROPERTY(bool priceAvailable READ priceAvailable)
    Q_PROPERTY(QString currency READ currency)
    Q_PROPERTY(Price * price READ price)

public:
    static PriceManager * instance(QNetworkAccessManager *manager);

    // Start the price polling thread
    Q_INVOKABLE bool start();
    // Stop the price polling thread
    Q_INVOKABLE bool stop();
    // Is there a price available yet?
    Q_INVOKABLE bool priceAvailable() const;
    // Are we running?
    Q_INVOKABLE bool running() const;
    // Are we refreshing the price?
    Q_INVOKABLE bool refreshing() const;
    // Get the current currency
    Q_INVOKABLE QString currency() const;
    // Get the current price
    Q_INVOKABLE Price *price() const;

    Q_INVOKABLE void handleError(const QString &msg) const;

private:
    void updatePrice(QNetworkReply *reply) const;

signals:
    void starting() const;
    void started() const;
    void priceRefreshStarted() const;
    void priceRefreshed() const;
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

private:
    explicit PriceManager(QNetworkAccessManager *manager, QObject *parent = 0);
    explicit PriceManager(QObject *parent = 0);
    static PriceManager * m_instance;
    mutable bool m_running;
    mutable bool m_refreshing;
    QNetworkAccessManager * m_manager;
    QTimer * m_timer;
    Price * m_price;

};

#endif // PRICEMANAGER_H
