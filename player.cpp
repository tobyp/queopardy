#include "player.h"

Player::Player(QObject *parent) : QObject(parent)
{

}

Player::Player(const QString &name, const QColor &color, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_color(color)
    , m_score(0)
{

}

QString Player::name() const
{
    return m_name;
}

QColor Player::color() const
{
    return m_color;
}

int Player::score() const
{
    return m_score;
}

int Player::key() const
{
    return m_key;
}

void Player::setName(QString name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(m_name);
}

void Player::setColor(QColor color)
{
    if (m_color == color)
        return;

    m_color = color;
    emit colorChanged(m_color);
}

void Player::setScore(int score)
{
    if (m_score == score)
        return;

    m_score = score;
    emit scoreChanged(m_score);
}

void Player::setKey(int key)
{
    if (m_key == key)
        return;

    m_key = key;
    emit keyChanged(m_key);
}
