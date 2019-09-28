#include "board.h"
#include "game.h"

#include <QJsonObject>

class Board::Private : public QObject
{
    Q_OBJECT

public:
    Private(Board *q)
    : q(q)
    {

    }

    Board *q;

    QList<QObject*> categories;
};

Board::Board(QObject *parent) : QObject(parent), d(new Private(this))
{

}

Board::~Board() {
    delete d;
}

QList<QObject *> Board::categories() const
{
    return d->categories;
}

void Board::loadJson(const QJsonArray &catArray, Game *game)
{
    for (QJsonValue const& catValue : catArray) {
        QJsonObject catObj = catValue.toObject();
        Category * cat = new Category(this);

        cat->setLabel(catObj.value("label").toString());

        int lastScore = 0;
        for (QJsonValue questionValue : catObj.value("questions").toArray()) {
            QJsonObject questionObject = questionValue.toObject();

            lastScore = questionObject.value("points").toInt(lastScore + 100);
            QString questionString = questionObject.value("question").toString();
            Question * question = new Question(lastScore, questionString);
            connect(question, &Question::questionStateChanged, this, &Board::questionStateChanged);

            bool revealed = questionObject.value("revealed").toBool();
            question->setRevealed(revealed);

            QJsonValue font = questionObject.value("font");
            if (!font.isUndefined()) {
                question->setFont(font.toString());
            }
            QJsonValue fontSize = questionObject.value("fontSize");
            if (!fontSize.isUndefined()) {
                question->setFontSize(fontSize.toInt());
            }
            for (QJsonValue paValue : questionObject.value("playerAnswers").toArray()) {
                QJsonObject paObj = paValue.toObject();
                QJsonValue playerValue = paObj.value("player");
                Player * player = nullptr;
                if (playerValue.isString()) {
                    QString playerName = paObj.value("player").toString();
                    player = game->getByName(playerName);
                }
                int score = paObj.value("score").toInt();
                question->addPlayerAnswer(player, score);
            }
            cat->addQuestion(question);
        }

        d->categories.append(cat);
    }

    emit categoriesChanged(d->categories);
}

QJsonArray Board::saveJson()
{
    QJsonArray catArray;
    for (auto const& catQObject : d->categories) {
        QJsonArray questionsArray;
        Category * cat = qobject_cast<Category*>(catQObject);
        for (auto const& qQ : cat->questions()) {
            Question * question = qobject_cast<Question*>(qQ);
            QJsonArray paArray;
            for (auto const& paQ : question->playerAnswers()) {
                PlayerAnswer * pa = qobject_cast<PlayerAnswer*>(paQ);
                QJsonObject paObj = {
                    {"player", pa->player() ? pa->player()->name() : QJsonValue()},
                    {"score", pa->score()},
                };
                paArray.append(paObj);
            }

            QJsonObject qObj = {
                {"points", question->points()},
                {"question", question->question()},
                {"playerAnswers", paArray},
                {"revealed", question->revealed()},
            };
            questionsArray.append(qObj);
        }
        QJsonObject catObj = {
            {"label", cat->label()},
            {"questions", questionsArray},
        };
        catArray.append(catObj);
    }
    return catArray;
}

void Board::reset()
{
    for (auto cat : d->categories) {
        delete cat;
    }
    d->categories.clear();
    emit categoriesChanged(d->categories);
}

void Board::setCategories(QList<QObject *> categories)
{
    if (d->categories == categories)
        return;

    d->categories = categories;
    emit categoriesChanged(d->categories);
}

void Board::questionStateChanged()
{
    emit boardStateChanged();
}

#include "board.moc"
