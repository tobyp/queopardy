#ifndef NETWORKPLAYERS_H
#define NETWORKPLAYERS_H

#include <QObject>
#include <QQmlParserStatus>
#include <QWebSocketServer>
#include <QPointer>

#include "game.h"

class NetworkPlayers : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(Game * game READ game WRITE setGame NOTIFY gameChanged)
    Q_PROPERTY(QString listenAddress READ listenAddress NOTIFY listenAddressChanged)

    QString m_listenAddress;

public:
    explicit NetworkPlayers(QObject *parent = nullptr);
    ~NetworkPlayers() override;

    Game * game() const;
    QString listenAddress() const;

    // QQmlParserStatus interface
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setGame(Game * game);

signals:
    void gameChanged(Game * game);
    void listenAddressChanged(QString listenAddress);

protected:
    class Private;
    Private *d;
};

#endif // NETWORKPLAYERS_H
