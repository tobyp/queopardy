#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtGlobal>
#include <QDir>
#include <QUrl>
#include <QDebug>

#include "category.h"
#include "question.h"
#include "player.h"
#include "networkplayers.h"
#include "board.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QString fileName = "board.json";
    if (argc >= 2) {
        fileName = argv[1];
    }

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterUncreatableType<Player>("com.queopardy", 1, 0, "Player", "Players can only be created by Game.addPlayer");
    qmlRegisterUncreatableType<Category>("com.queopardy", 1, 0, "Category", "Categories can only be loaded");
    qmlRegisterUncreatableType<Question>("com.queopardy", 1, 0, "Category", "Questions can only be loaded");
    qmlRegisterUncreatableType<PlayerAnswer>("com.queopardy", 1, 0, "PlayerAnswer", "PlayerAnswers can only be created by Question.addPlayerAnswer");
    qmlRegisterUncreatableType<Question>("com.queopardy", 1, 0, "Category", "Questions can only be loaded");
    qmlRegisterUncreatableType<Game>("com.queopardy", 1, 0, "Game", "Game is only available as the context property ctxGame");
    qmlRegisterType<NetworkPlayers>("com.queopardy", 1, 0, "NetworkPlayers");

    Game * game = new Game();
    game->load(fileName);

    QDir dir(argv[1]);
    dir.cdUp();
    engine.rootContext()->setContextProperty("resourceUrl", QUrl::fromLocalFile(dir.absolutePath()));
    engine.rootContext()->setContextProperty("ctxGame", game);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (url == objUrl) {
            if (!obj)
                QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
