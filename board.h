#ifndef BOARD_H
#define BOARD_H

#include <QObject>
#include <QJsonArray>
#include "category.h"
#include "playeranswer.h"

class Game;

class Board : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> categories READ categories WRITE setCategories NOTIFY categoriesChanged)
public:
    explicit Board(QObject *parent = nullptr);
    ~Board() override;

    QList<QObject*> categories() const;

    void loadJson(QJsonArray const& catArray, Game * game);
    QJsonArray saveJson();
    void reset();

signals:
    void categoriesChanged(QList<QObject*> categories);
    void boardStateChanged();

public slots:

    void setCategories(QList<QObject*> categories);
    void questionStateChanged();

private:
    class Private;
    Private * d;
};

#endif // BOARD_H
