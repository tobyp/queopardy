#ifndef PLAYERANSWER_H
#define PLAYERANSWER_H

#include <QObject>

#include "player.h"

class Question;

class PlayerAnswer : public QObject {
	Q_OBJECT

	Q_PROPERTY(Player * player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged)

public:
	explicit PlayerAnswer(QObject * parent = nullptr);
	explicit PlayerAnswer(Player * player, int score, QObject * parent = nullptr);

	Player * player() const;
	int score() const;
	Question * question() const;

signals:
	void playerChanged(Player * player);
	void scoreChanged(int score);

public slots:
	void setPlayer(Player * player);
	void setScore(int score);

private:
	Player * m_player;
	int m_score;
};

#endif  // PLAYERANSWER_H
