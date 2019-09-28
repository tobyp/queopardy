#include "game.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QtDebug>
#include <cmath>
#include <QDateTime>

#include "category.h"
#include "player.h"
#include "playeranswer.h"

class Game::Private : public QObject
{
    Q_OBJECT

public:
    Private(Game *q)
    : q(q)
    , board(new Board(q))
    , state(GameState::LOBBY)
    , minPlayers(2)
    , maxPlayers(4)
    {
        connect(board, &Board::boardStateChanged, this, &Game::Private::saveBackup);
    }

    QColor nextFreePlayerColor();

    Game *q;

    QString baseFileName;
    Board * board;
    GameState state;
    int minPlayers;
    int maxPlayers;
    QList<Player *> players;
    bool loading = false;

public slots:
    void saveBackup();
    void onPlayerColorChanged(QColor const& color);
};

Game::Game(QObject *parent) : QAbstractListModel(parent), d(new Private(this))
{

}

Game::~Game()
{
    delete d;
}

QColor parseColor(QJsonValue const& value) {
    QJsonObject colorObj = value.toObject();
    int red = colorObj.value("red").toInt();
    int green = colorObj.value("green").toInt();
    int blue = colorObj.value("blue").toInt();
    return QColor(red, green, blue);
}

QJsonValue unparseColor(QColor const& color) {
    return QJsonObject {
        {"red", color.red()},
        {"green", color.green()},
        {"blue", color.blue()},
    };
}

void Game::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCritical("Failed to open board file: %s", fileName.toUtf8().data());
        return;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (doc.isNull()) {
        qCritical("Failed to parse board file: %s: %s", fileName.toUtf8().data(), error.errorString().toUtf8().data());
        return;
    }

    if (!doc.isObject()) {
        qCritical("Document is not an Object");
        return;
    }

    QJsonObject docObject = doc.object();

    d->loading = true;
    reset();

    QJsonArray playersArray = docObject.value("players").toArray();
    for (QJsonValue playerValue : playersArray) {
        QJsonObject playerObject = playerValue.toObject();
        QString playerName = playerObject.value("name").toString();
        addPlayer(playerName, parseColor(playerObject.value("color").toObject()));  //this won't add if it already exists
    }

    d->board->loadJson(docObject.value("categories").toArray(), this);
    d->baseFileName = fileName.chopped(5);  // cut off json

    d->loading = false;
}

void Game::save(const QString &fileName)
{
    QJsonArray catArray = d->board->saveJson();

    QJsonArray playersArray;
    for (Player * player : d->players) {
        QJsonObject playerObj = {
          {"name", player->name()},
          {"color", unparseColor(player->color())},
        };
        playersArray.append(playerObj);
    }

    QJsonObject gameObj{
        {"categories", catArray},
        {"players", playersArray},
    };

    QJsonDocument doc(gameObj);

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qCritical() << "Couldn't open" << fileName << "for writing";
        return;
    }

    file.write(doc.toJson());
    file.close();
}

void Game::reset()
{
    d->board->reset();
    for (auto player : d->players) {
        player->setScore(0);
    }
}

Board *Game::board() const
{
    return d->board;
}

void Game::setBoard(Board *board)
{
    if (d->board == board)
        return;

    d->board = board;
    emit boardChanged(d->board);
}

void Game::setState(Game::GameState state)
{
    if (d->state == state) {
        return;
    }

    d->state = state;

    emit minPlayersChanged(state);
}

void Game::setMinPlayers(int minPlayers)
{
    if (d->minPlayers == minPlayers) {
        return;
    }

    d->minPlayers = minPlayers;

    emit minPlayersChanged(minPlayers);
}

int Game::minPlayers() const
{
    return d->minPlayers;
}

void Game::setMaxPlayers(int maxPlayers)
{
    if (d->maxPlayers == maxPlayers) {
        return;
    }

    d->maxPlayers = maxPlayers;

    emit maxPlayersChanged(maxPlayers);
}

int Game::maxPlayers() const
{
    return d->maxPlayers;
}

Player *Game::get(int idx) const
{
    if (idx < 0 || idx >= d->players.size()) {
        return nullptr;
    }

    Player *player = d->players.at(idx);

    return player;
}

Player *Game::getByName(const QString &name)
{
    for (auto p : d->players) {
        if (p->name() == name) return p;
    }
    return nullptr;
}

Player *Game::addPlayerAutocolor(const QString &name, bool createPlayer)
{
    return addPlayer(name, d->nextFreePlayerColor(), createPlayer);
}

Player *Game::addPlayer(const QString &name, const QColor &color, bool createPlayer)
{
    for(Player *p : d->players) {
        if (p->name() == name) {
            return p;
        }
    }

    if (!createPlayer) return nullptr;

    Player *player = new Player(name, color, this);
    connect(player, &Player::colorChanged, d, &Game::Private::onPlayerColorChanged);

    int pos = d->players.size();
    beginInsertRows(QModelIndex(), pos, pos);
    d->players.append(player);
    endInsertRows();

    emit countChanged(d->players.size());

    return player;
}

void Game::removePlayer(Player *player)
{
    int idx = d->players.indexOf(player);
    if (idx != -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        d->players.removeAt(idx);
        endRemoveRows();

        emit countChanged(d->players.size());
    }
}

void Game::buzz(Player *player)
{
    emit playerBuzzed(player);
}

int Game::count() const
{
    return d->players.size();
}

QHash<int, QByteArray> Game::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(NameRole, "name");
    roleNames.insert(ColorRole, "color");
    roleNames.insert(ScoreRole, "score");
    roleNames.insert(KeyRole , "key");
    return roleNames;
}

int Game::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->players.size();
}

QVariant Game::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row < 0 || row >= d->players.size()) {
        return QVariant();
    }

    const Player *player = d->players.at(row);

    switch(static_cast<Role>(role)) {
    case Game::NameRole:
        return player->name();
    case Game::ColorRole:
        return player->color();
    case Game::ScoreRole:
        return player->score();
    case Game::KeyRole:
        return player->key();
    }

    return QVariant();
}

Game::GameState Game::state() const
{
    return d->state;
}

QColor Game::Private::nextFreePlayerColor()
{
    QColor color;
    for (int i=0; i<players.size() + 1; ++i) {
        // https://gamedev.stackexchange.com/questions/46463/how-can-i-find-an-optimum-set-of-colors-for-10-players
        color = QColor::fromHsvF(fmod(i * 0.618033988749895, 1.0), 0.5, 1.0);
//        color = QColor::fromRgb((i * 158) % 256, 127, 255);
        if (std::find_if(players.cbegin(), players.cend(), [&color](Player *const p){ return p->color() == color; }) == players.cend()) {
            break;
        }
    }
    return color;
}

void Game::Private::saveBackup()
{
    if (loading)
        return;

    QDateTime now = QDateTime::currentDateTime();
    QString backupFileName = baseFileName + "." + now.toString("yyyy-MM-ddTHH-mm-ss") + ".json";
    q->save(backupFileName);
}

void Game::Private::onPlayerColorChanged(const QColor &color)
{
    Q_UNUSED(color);
    Player * player = qobject_cast<Player*>(sender());
    int idx = players.indexOf(player);
    emit q->dataChanged(q->index(idx), q->index(idx), {Role::ColorRole});
}

#include "game.moc"
