#ifndef QUESTION_H
#define QUESTION_H

#include <QObject>

#include "playeranswer.h"

class Question : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString question READ question WRITE setQuestion NOTIFY questionChanged)
    Q_PROPERTY(int points READ points WRITE setPoints NOTIFY pointsChanged)
    Q_PROPERTY(bool revealed READ revealed WRITE setRevealed NOTIFY revealedChanged)
    Q_PROPERTY(QString font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QList<QObject*> playerAnswers READ playerAnswers WRITE setPlayerAnswers NOTIFY playerAnswersChanged)

public:
    explicit Question(QObject *parent = nullptr);
    explicit Question(int points, QString const& question, QObject *parent = nullptr);

    QString question() const;

    int points() const;

    bool revealed() const;

    QString font() const;

    int fontSize() const;

    QList<QObject*> playerAnswers() const;

signals:
    void questionChanged(QString question);

    void pointsChanged(int points);

    void revealedChanged(bool revealed);

    void fontChanged(QString font);

    void fontSizeChanged(int fontSize);

    void playerAnswersChanged(QList<QObject*> playerAnswers);

    void questionStateChanged();

public slots:
    void setQuestion(QString question);

    void setPoints(int points);

    void setRevealed(bool revealed);

    void setFont(QString font);

    void setFontSize(int fontSize);

    void setPlayerAnswers(QList<QObject*> playerAnswers);

    void addPlayerAnswer(QObject * player, int score);

    void playerAnswerStateChanged();

private:
    int m_points;
    QString m_question;
    bool m_revealed;
    QString m_font;
    int m_fontSize;
    QList<QObject*> m_playerAnswers;
    QObject * m_winner;
};

#endif // QUESTION_H
