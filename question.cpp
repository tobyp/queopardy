#include "player.h"
#include "playeranswer.h"
#include "question.h"

Question::Question(QObject *parent)
    : QObject(parent)
    , m_points(0)
    , m_revealed(false)
    , m_fontSize(40)
{

}

Question::Question(int points, const QString &question, QObject *parent)
    : QObject(parent)
    , m_points(points)
    , m_question(question)
    , m_revealed(false)
    , m_fontSize(40)
{

}

QString Question::question() const
{
    return m_question;
}

int Question::points() const
{
    return m_points;
}

bool Question::revealed() const
{
    return m_revealed;
}

QString Question::font() const
{
    return m_font;
}

int Question::fontSize() const
{
    return m_fontSize;
}

QList<QObject *> Question::playerAnswers() const
{
    return m_playerAnswers;
}

void Question::setQuestion(QString question)
{
    if (m_question == question)
        return;

    m_question = question;
    emit questionChanged(m_question);
}

void Question::setPoints(int points)
{
    if (m_points == points)
        return;

    m_points = points;
    emit pointsChanged(m_points);
}

void Question::setRevealed(bool revealed)
{
    if (m_revealed == revealed)
        return;

    m_revealed = revealed;
    emit revealedChanged(m_revealed);
}

void Question::setFont(QString font)
{
    if (m_font == font)
        return;

    m_font = font;
    emit fontChanged(m_font);
}

void Question::setFontSize(int fontSize)
{
    if (m_fontSize == fontSize)
        return;

    m_fontSize = fontSize;
    emit fontSizeChanged(m_fontSize);
}

void Question::setPlayerAnswers(QList<QObject *> playerAnswers)
{
    if (m_playerAnswers == playerAnswers)
        return;

    m_playerAnswers = playerAnswers;
    emit playerAnswersChanged(m_playerAnswers);
}

void Question::addPlayerAnswer(QObject *player, int score)
{
    Player * p = qobject_cast<Player*>(player);
    if (p) p->setScore(p->score() + score);
    PlayerAnswer * pa = new PlayerAnswer(p, score, this);
    m_playerAnswers.append(pa);
    connect(pa, &PlayerAnswer::playerChanged, this, &Question::playerAnswerStateChanged);
    connect(pa, &PlayerAnswer::scoreChanged, this, &Question::playerAnswerStateChanged);
    emit playerAnswersChanged(m_playerAnswers);
    emit questionStateChanged();
}

void Question::playerAnswerStateChanged()
{
    emit questionStateChanged();
}
