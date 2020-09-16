#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QColor>

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged)
    Q_PROPERTY(int key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit Player(QObject * parent = nullptr);
    explicit Player(int id, QString const& name, QColor const& color, QObject * parent = nullptr);

    int id() const;
    QString name() const;
    QColor color() const;
    int score() const;
    int key() const;
    bool active() const;

    QString token() const;
    void setToken(QString const& token);

public slots:
    void setId(int id);
    void setName(QString const& name);
    void setColor(QColor const& color);
    void setScore(int score);
    void setKey(int key);
    void setActive(bool active);

signals:
    void idChanged(int id);
    void nameChanged(QString const& name);
    void colorChanged(QColor const& color);
    void scoreChanged(int score);
    void keyChanged(int key);
    void activeChanged(bool active);

private:
    int m_id;
    QString m_name;
    QColor m_color;
    QString m_token;
    int m_score = 0;
    int m_key = 0;
    bool m_active = false;
};

#endif // PLAYER_H
