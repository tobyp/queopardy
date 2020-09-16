#ifndef CATEGORY_H
#define CATEGORY_H

#include "question.h"

#include <QObject>
#include <QColor>

class Category : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QList<QObject*> questions READ questions WRITE setQuestions NOTIFY questionsChanged)

public:
    explicit Category(QObject *parent = nullptr);

    QString label() const;

    QList<QObject*> questions() const;
    int getTileId(Question * question) const;

    void addQuestion(Question * question);

signals:
    void labelChanged(QString label);
    void questionsChanged(QList<QObject*> questions);

public slots:
    void setLabel(QString label);

    void setQuestions(QList<QObject*> questions);

private:
    QString m_label;
    QList<QObject*> m_questions;
};

#endif // CATEGORY_H
