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
    Q_PROPERTY(int minPlayers READ minPlayers WRITE setMinPlayers NOTIFY minPlayersChanged)
    Q_PROPERTY(int maxPlayers READ maxPlayers WRITE setMaxPlayers NOTIFY maxPlayersChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool joinable READ joinable WRITE setJoinable NOTIFY joinableChanged)
    Q_PROPERTY(Question * openQuestion READ openQuestion WRITE setOpenQuestion NOTIFY openQuestionChanged)
    Q_PROPERTY(Player * buzzerPlayer READ buzzerPlayer WRITE setBuzzerPlayer NOTIFY buzzerPlayerChanged)
    Q_PROPERTY(Player * turnPlayer READ turnPlayer WRITE setTurnPlayer NOTIFY turnPlayerChanged)

public:
    enum QuestionResolution {
        CORRECT,
        INCORRECT,
        FAIL,
    };
    Q_ENUM(QuestionResolution);

    explicit Game(QObject *parent = nullptr);
    ~Game() override;

    Board * board() const;
    int minPlayers() const;
    int maxPlayers() const;
    int count() const;
    bool joinable() const;
    Question * openQuestion() const;
    Player * buzzerPlayer() const;
    Player * turnPlayer() const;

    Q_INVOKABLE void load(QString const& fileName);
    Q_INVOKABLE void save(QString const& fileName);
    void reset();

    Q_INVOKABLE Player *get(int idx) const;
    Q_INVOKABLE int indexOf(Player *player) const;
    Player *getByName(QString const& name);
    Player *getByToken(QString const& token);
    Q_INVOKABLE Player * addPlayer(int id, const QString &name, QColor const& color);
    Q_INVOKABLE bool removePlayer(Player * player);

    int getCategoryId(Category * category) const;

    Q_INVOKABLE bool buzz(Player * player);
    Q_INVOKABLE void resolve(Question * question, Player * player, QuestionResolution resolution);

    Q_INVOKABLE int nextFreePlayerId();
    Q_INVOKABLE QString nextFreePlayerName();
    Q_INVOKABLE QColor nextFreePlayerColor();

    // QAbstractItemModel interface
    enum Role {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        ScoreRole,
        KeyRole,
        ActiveRole,
    };
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

signals:
    void boardChanged(Board * board);
    void minPlayersChanged(int minPlayers);
    void maxPlayersChanged(int maxPlayers);
    void countChanged(int count);
    void joinableChanged(bool joinable);
    void openQuestionChanged(Question * openQuestion);
    void openQuestionResolve(PlayerAnswer * answer);
    void buzzerPlayerChanged(Player * buzzerPlayer);
    void turnPlayerChanged(Player * buzzerPlayer);
    void playerActiveChanged(Player * player);
    void playerAdded(Player * player);
    void playerRemoved(Player * player);

public slots:
    void setBoard(Board * board);
    void setMinPlayers(int minPlayers);
    void setMaxPlayers(int maxPlayers);
    void setJoinable(bool joinable);
    void setOpenQuestion(Question * openQuestion);
    void setBuzzerPlayer(Player * buzzerPlayer);
    void setTurnPlayer(Player * buzzerPlayer);


private:
    class Private;
    Private *d;
};

#endif // GAME_H
