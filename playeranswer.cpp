#include "playeranswer.h"
#include "question.h"

PlayerAnswer::PlayerAnswer(QObject *parent)
    : QObject(parent), m_player(nullptr), m_score(0)
{

}

PlayerAnswer::PlayerAnswer(Player *player, int score, QObject *parent) : QObject(parent), m_player(player), m_score(score)
{

}

Player *PlayerAnswer::player() const
{
    return m_player;
}

int PlayerAnswer::score() const
{
    return m_score;
}

Question * PlayerAnswer::question() const
{
    return qobject_cast<Question *>(parent());
}

void PlayerAnswer::setPlayer(Player *player)
{
    if (m_player == player)
        return;

    m_player = player;
    emit playerChanged(m_player);
}

void PlayerAnswer::setScore(int score)
{
    if (m_score == score)
        return;

    m_score = score;
    emit scoreChanged(m_score);
}
