#include "networkplayers.h"
#include "player.h"

#include <QQmlEngine>
#include <QLoggingCategory>
#include <QWebSocket>


Q_LOGGING_CATEGORY(logNetworkPlayers, "com.queopardy.networkplayers");

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QWebSocket *socket, QObject *parent = nullptr)
    : QObject(parent)
    , socket(socket)
    {
        connect(socket, &QWebSocket::textMessageReceived, this, &NetworkClient::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, this, &NetworkClient::onDisconnect);
    }

    void send(const QString &text) {
        socket->sendTextMessage(text);
    }

    QString connectionString() const {
        return QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    }

private slots:
    void onTextMessageReceived(const QString &message) {
        QString msg = message.trimmed();

        NetworkPlayers * players = qobject_cast<NetworkPlayers *>(parent()->parent());
        Game::GameState state = players->game()->state();
        if (msg.startsWith("hello ")) {
            QString name = msg.mid(6);
            if (name.isEmpty()) {
                send("not welcome name is empty");
            } else {
                // Signalize we have a new player with this name
                emit nameChanged(name);

                qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is controlling" << name;

                send("welcome");
            }
        }
        else if (msg.startsWith("color ")) {
            QStringList args = msg.mid(6).split(" ");
            if (args.length() == 3) {
                bool rok, gok, bok;
                int red = args[0].toInt(&rok);
                int green = args[1].toInt(&gok);
                int blue = args[2].toInt(&bok);
                if (!rok || !gok || !bok || red < 0|| green < 0 || blue < 0 || red > 255 || green > 255 || blue > 255) {
                    send("error incorrect syntax; 'color red green blue' with rgb values between 0 and 255");
                }
                else {
                    if (player) {
                        player->setColor(QColor(red, green, blue));
                        send("ok your color was changed successfully");
                    }
                    else {
                        send("error you're not registered yet, send a 'hello' command first");
                    }
                }
            }
            else {
                send("error incorrect syntax; 'color red green blue' with rgb values between 0 and 255");
            }
        }
        else if (msg == "buzz") {
            if (state == Game::BUZZABLE) {
                if (player) {
                    players->game()->buzz(player);
                }
                else {
                    send("error you're not registered yet, send a 'hello' command first");
                }
            }
            else {
                send("error the buzzer is currently locked.");
            }
        }
        else {
            send("error unrecognized command, try 'hello', 'color', or 'buzz'");
        }
    }

    void onDisconnect() {
        emit disconnected();
    }

signals:
    void buzzed();
    void nameChanged(const QString &name);
    void disconnected();

public:
    Player *player = nullptr;
    QWebSocket *socket = nullptr;
};

class NetworkPlayers::Private : public QObject
{
    Q_OBJECT

public:
    Private(NetworkPlayers *q)
    : QObject(q)
    , q(q)
    , server("jeopardy", QWebSocketServer::NonSecureMode, this)
    {
        connect(&server, &QWebSocketServer::newConnection, this, &NetworkPlayers::Private::onNewConnection);
    }

    NetworkPlayers *q;

    Game * game;

    // Properties
    QWebSocketServer server;
    QList<NetworkClient *> clients;

public slots:
    void onNewConnection();
    void onClientNameChanged(const QString &name);
    void onClientBuzzed();
    void onClientDisconnected();
};

void NetworkPlayers::Private::onNewConnection()
{
    QWebSocket *socket = server.nextPendingConnection();

    NetworkClient *client = new NetworkClient(socket, this);
    connect(client, &NetworkClient::nameChanged, this, &NetworkPlayers::Private::onClientNameChanged);
    connect(client, &NetworkClient::buzzed, this, &NetworkPlayers::Private::onClientBuzzed);
    connect(client, &NetworkClient::disconnected, this, &NetworkPlayers::Private::onClientDisconnected);
    qInfo(logNetworkPlayers) << "New connection from" << client->connectionString();

    clients.append(client);
}

void NetworkPlayers::Private::onClientNameChanged(const QString &name)
{
    NetworkClient *client = qobject_cast<NetworkClient *>(sender());
    Player * newPlayer = q->game()->addPlayerAutocolor(name, q->game()->state() == Game::LOBBY);
    if (newPlayer == nullptr) {
        client->send("error you can't join a running game with a new player");
        return;
    }
    if (std::find_if(clients.cbegin(), clients.cend(), [&newPlayer](NetworkClient *client){ return client->player == newPlayer; }) != clients.cend()) {
        client->send("error that player is already controlled by someone");
        return;
    }
    if (client->player) {
        q->game()->removePlayer(client->player);
    }
    client->player = newPlayer;
}

void NetworkPlayers::Private::onClientBuzzed()
{
    NetworkClient *client = qobject_cast<NetworkClient *>(sender());
    if (client->player) {
        q->game()->buzz(client->player);
    }
    else {
        client->send("error not attached to a player");
    }
}

void NetworkPlayers::Private::onClientDisconnected() {
    NetworkClient *client = qobject_cast<NetworkClient *>(sender());
    clients.removeOne(client);
    qInfo(logNetworkPlayers) << "Lost connection from" << client->connectionString();
    delete client;
}


NetworkPlayers::NetworkPlayers(QObject *parent)
: QObject(parent)
, d(new Private(this))
{
}

NetworkPlayers::~NetworkPlayers()
{
    //delete d; //parent relation does this
}

void NetworkPlayers::classBegin()
{
    if (!d->server.listen(QHostAddress::Any, 31337)) {
        qCCritical(logNetworkPlayers) << "Unable to listen:" << d->server.errorString();
    }
    else {
        qInfo(logNetworkPlayers) << "Listening on" << d->server.serverUrl().toString();
    }
}

void NetworkPlayers::componentComplete()
{

}

QString NetworkPlayers::listenAddress() const
{
    return d->server.serverUrl().toString();
}

void NetworkPlayers::setGame(Game *game)
{
    if (d->game == game)
        return;

    d->game = game;
    emit gameChanged(d->game);
}

Game *NetworkPlayers::game() const
{
    return d->game;
}

#include "networkplayers.moc"
