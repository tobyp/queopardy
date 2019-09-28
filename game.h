#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QAbstractListModel>

#include "board.h"
#include "player.h"

class Game : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Board * board READ board WRITE setBoard NOTIFY boardChanged)
    Q_PROPERTY(GameState state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(int minPlayers READ minPlayers WRITE setMinPlayers NOTIFY minPlayersChanged)
    Q_PROPERTY(int maxPlayers READ maxPlayers WRITE setMaxPlayers NOTIFY maxPlayersChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum GameState {
        LOBBY,
        BUZZABLE,
        UNBUZZABLE
    };
    Q_ENUM(GameState)

    explicit Game(QObject *parent = nullptr);
    ~Game() override;

    Board * board() const;
    GameState state() const;
    int minPlayers() const;
    int maxPlayers() const;
    int count() const;

    Q_INVOKABLE void load(QString const& fileName);
    Q_INVOKABLE void save(QString const& fileName);
    void reset();

    Q_INVOKABLE Player *get(int idx) const;
    Player *getByName(QString const& name);
    Q_INVOKABLE Player * addPlayerAutocolor(const QString &name, bool createPlayer=true);
    Q_INVOKABLE Player * addPlayer(const QString &name, QColor const& color, bool createPlayer=true);
    Q_INVOKABLE void removePlayer(Player * player);

    Q_INVOKABLE void buzz(Player * player);

    // QAbstractItemModel interface
    enum Role {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        ScoreRole,
        KeyRole
    };
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;


signals:
    void boardChanged(Board * board);
    void stateChanged(GameState state);
    void minPlayersChanged(int minPlayers);
    void maxPlayersChanged(int maxPlayers);
    void countChanged(int count);

    void playerBuzzed(Player *player);

public slots:
    void setBoard(Board * board);
    void setState(GameState state);
    void setMinPlayers(int minPlayers);
    void setMaxPlayers(int maxPlayers);

private:
    class Private;
    Private *d;
};

#endif // GAME_H
