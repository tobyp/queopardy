#include "player.h"

Player::Player(QObject *parent) : QObject(parent)
{

}

Player::Player(int id, const QString &name, const QColor &color, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_color(color)
    , m_score(0)
{

}

int Player::id() const
{
    return m_id;
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

bool Player::active() const
{
    return m_active;
}

QString Player::token() const {
    return m_token;
}

void Player::setToken(QString const& token) {
    m_token = token;
}

void Player::setId(int id)
{
    if (m_id == id)
        return;

    m_id = id;
    emit idChanged(m_id);
}

void Player::setName(QString const& name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(m_name);
}

void Player::setColor(QColor const& color)
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

void Player::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    emit activeChanged(m_active);
}
