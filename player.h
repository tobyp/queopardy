#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QColor>

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged)
    Q_PROPERTY(int key READ key WRITE setKey NOTIFY keyChanged)

public:
    explicit Player(QObject * parent = nullptr);
    explicit Player(QString const& name, QColor const& color, QObject * parent = nullptr);

    QString name() const;

    QColor color() const;

    int score() const;

    int key() const;

public slots:
    void setName(QString name);

    void setColor(QColor color);

    void setScore(int score);

    void setKey(int key);

signals:
    void nameChanged(QString name);

    void colorChanged(QColor color);

    void scoreChanged(int score);

    void keyChanged(int key);

private:
    QString m_name;
    QColor m_color;
    int m_score = 0;
    int m_key = 0;
};

#endif // PLAYER_H
