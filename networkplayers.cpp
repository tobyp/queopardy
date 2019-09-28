#include "networkplayers.h"
#include "player.h"

#include <QQmlEngine>
#include <QLoggingCategory>
#include <QWebSocket>


Q_LOGGING_CATEGORY(logNetworkPlayers, "com.queopardy.networkplayers");

class NetworkClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Game * game READ game WRITE setGame NOTIFY gameChanged)

public:
    explicit NetworkClient(Game * game, QWebSocket *socket, QObject *parent = nullptr)
    : QObject(parent)
    , m_socket(socket)
    , m_game(game)
    {
        connect(socket, &QWebSocket::textMessageReceived, this, &NetworkClient::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, this, &NetworkClient::onDisconnect);
        connect(game, &Game::openQuestionChanged, this, &NetworkClient::onQuestion);
        connect(game, &Game::buzzerPlayerChanged, this, &NetworkClient::onBuzzer);
        m_player = game->addPlayer(game->nextFreePlayerName(), game->nextFreePlayerColor()); //null if the game is not joinable
    }

    void send(const QString &text) {
        m_socket->sendTextMessage(text);
    }

    QString connectionString() const {
        return QString("%1:%2").arg(m_socket->peerAddress().toString()).arg(m_socket->peerPort());
    }

    Game * game() const {
        return m_game;
    }

private slots:
    void onTextMessageReceived(const QString &message) {
        QString msg = message.trimmed();

        if (msg.startsWith("hello ")) {
            onCommandName(msg.mid(6));
        }
        else if (msg.startsWith("color ")) {
            onCommandColor(msg.mid(6));
        }
        else if (msg == "buzz") {
            onCommandBuzz();
        }
        else if (msg == "nextColor") {
            onCommandNextColor();
        }
        else {
            send("error unrecognized command, try 'hello', 'color', or 'buzz'");
        }
    }

    void onQuestion(Question * question) {
        if (question == nullptr) {
            send("question closed");
        }
        else {
            if (question->question().startsWith("[img]")) {
                send("question open <image>");
            }
            else {
                send("question open " + question->question());
                send("buzzer open");
            }
        }
    }

    void onBuzzer(Player * player) {
        if (player == nullptr) {
            send("buzzer open");
        }
        else {
            send("buzzer closed " + player->name());
        }
    }

    void onDisconnect() {
        emit disconnected();
    }

private:
    void onCommandBuzz() {
        if (!m_player) {
            send("error you're not registered yet, send a 'hello' command first");
            return;
        }

        if (m_game->joinable()) {
            send("error you can't buzz now, the game hasn't started!");
            return;
        }

        if (m_game->openQuestion() == nullptr) {
            send("error you can't buzz now, there's no open question!");
            return;
        }

        if (m_game->buzzerPlayer() != nullptr) {
            send("error you can't buzz now, someone else has already buzzed: " + m_game->buzzerPlayer()->name());
            return;
        }

        bool gotIt = m_game->buzz(m_player);
        if (gotIt) {
            send("ok you buzzed!");
        }
        else {
            send("error buzzing failed");
        }
    }
    void onCommandColor(QString const& argsString) {
        QStringList args = argsString.split(" ");
        if (args.length() != 3) {
            send("error incorrect syntax; 'color red green blue' with rgb values between 0 and 255");
            return;
        }

        bool rok, gok, bok;
        int red = args[0].toInt(&rok);
        int green = args[1].toInt(&gok);
        int blue = args[2].toInt(&bok);
        if (!rok || !gok || !bok || red < 0|| green < 0 || blue < 0 || red > 255 || green > 255 || blue > 255) {
            send("error incorrect syntax; 'color red green blue' with rgb values between 0 and 255");
            return;
        }
        if (!m_player) {
            send("error you're not registered yet, send a 'hello' command first");
            return;
        }
        m_player->setColor(QColor(red, green, blue));
        send("ok your color was changed successfully");
    }

    void onCommandName(QString const& name) {
        if (name.isEmpty()) {
            send("error name is empty");
            return;
        }

        emit nameChanged(name);
    }

    void onCommandNextColor() {
        if (!m_player) {
            send("error you're not registered yet, send a 'hello' command first");
            return;
        }

        m_player->setColor(m_game->nextFreePlayerColor());
        send("ok your color was changed!");
    }

signals:
    void nameChanged(const QString &name);
    void gameChanged(Game * m_game);
    void disconnected();

public:
    Player *m_player = nullptr;
    QWebSocket *m_socket = nullptr;
    Game * m_game = nullptr;

public slots:
    void setGame(Game * game)
    {
        if (m_game == game)
            return;

        m_game = game;
        emit gameChanged(m_game);
    }
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
        connect(&server, &QWebSocketServer::newConnection, this, &NetworkPlayers::Private::onClientConnected);
    }

    NetworkPlayers *q;

    Game * game;

    // Properties
    QWebSocketServer server;
    QList<NetworkClient *> clients;

    NetworkClient * getController(Player * player);
public slots:
    void onClientConnected();
    void onClientNameChanged(const QString &name);
    void onClientDisconnected();
};

NetworkClient *NetworkPlayers::Private::getController(Player *player)
{
    auto it = std::find_if(clients.cbegin(), clients.cend(), [&player](auto client){ return client->m_player == player; });
    if (it == clients.cend()) return nullptr;
    return *it;
}

void NetworkPlayers::Private::onClientConnected()
{
    QWebSocket *socket = server.nextPendingConnection();

    NetworkClient *client = new NetworkClient(game, socket, this);
    connect(client, &NetworkClient::nameChanged, this, &NetworkPlayers::Private::onClientNameChanged);
    connect(client, &NetworkClient::disconnected, this, &NetworkPlayers::Private::onClientDisconnected);
    qInfo(logNetworkPlayers) << "New connection from" << client->connectionString();

    clients.append(client);
}

void NetworkPlayers::Private::onClientNameChanged(const QString &name)
{
    NetworkClient *client = qobject_cast<NetworkClient *>(sender());
    Player * existingPlayer = q->game()->getByName(name);

    if (existingPlayer) {  // target is another player
        NetworkClient * existingController = getController(existingPlayer);
        if (existingController) {
            client->send("error that player is already controlled by another client");
            return;
        }

        if (client->m_player) {
            qCDebug(logNetworkPlayers) << "Connection" << client->connectionString() << "is NO LONGER controlling" << client->m_player->name();
            q->game()->removePlayer(client->m_player);
        }
        client->m_player = existingPlayer;
        client->send("welcome");
        emit game->playerJoined(client->m_player);
        qCDebug(logNetworkPlayers) << "Connection" << client->connectionString() << "is now controlling" << client->m_player->name();

    }
    else if (client->m_player) {  // target is self
        QString oldName = client->m_player->name();
        client->m_player->setName(name);
        client->send("welcome");
        qCDebug(logNetworkPlayers) << "Player" << oldName << "is now" << name;

        if (!client->m_player->property("soundPlayed").toBool()) {
            emit game->playerJoined(client->m_player);
            client->m_player->setProperty("soundPlayed", true);
        }
    }
    else {  // target is a new player
        Player * newPlayer = q->game()->addPlayer(name, game->nextFreePlayerColor());
        if (newPlayer) {
            client->m_player = newPlayer;
            emit game->playerJoined(client->m_player);
            client->send("welcome");
            qCDebug(logNetworkPlayers) << "Connection" << client->connectionString() << "is now controlling" << client->m_player->name();

            client->send("welcome");
        }
        else {
            client->send("error you cannot join a running game");
            return;
        }
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
