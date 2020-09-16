#include "category.h"

Category::Category(QObject *parent) : QObject(parent)
{

}

QString Category::label() const
{
    return m_label;
}

QList<QObject *> Category::questions() const
{
    return m_questions;
}

int Category::getTileId(Question * question) const {
    return m_questions.indexOf(question);
}

void Category::addQuestion(Question *question)
{
    m_questions.append(question);
}

void Category::setLabel(QString label)
{
    if (m_label == label)
        return;

    m_label = label;
    emit labelChanged(m_label);
}

void Category::setQuestions(QList<QObject *> questions)
{
    if (m_questions == questions)
        return;

    m_questions = questions;
    emit questionsChanged(m_questions);
}
