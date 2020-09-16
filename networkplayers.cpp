#include "networkplayers.h"

#include <QLoggingCategory>
#include <QQmlEngine>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtCore/QFile>

#include "player.h"

Q_LOGGING_CATEGORY(logNetworkPlayers, "com.queopardy.networkplayers");

class NetworkClient : public QObject {
	Q_OBJECT

	Q_PROPERTY(Game * game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(Player * player READ player WRITE setPlayer NOTIFY playerChanged)

	public:
	explicit NetworkClient(Game * game, QWebSocket * socket, QObject * parent = nullptr)
		: QObject(parent), m_socket(socket), m_game(game) {
		connect(socket, &QWebSocket::textMessageReceived, this, &NetworkClient::onTextMessageReceived);
		connect(socket, &QWebSocket::disconnected, this, &NetworkClient::onDisconnect);
		connect(game, &Game::openQuestionChanged, this, &NetworkClient::onQuestion);
		connect(game, &Game::openQuestionResolve, this, &NetworkClient::onResolve);
		connect(game, &Game::buzzerPlayerChanged, this, &NetworkClient::onBuzzer);
		connect(game, &Game::turnPlayerChanged, this, &NetworkClient::onTurn);
		connect(game, &Game::playerAdded, this, &NetworkClient::onPlayerAdded);
		connect(game, &Game::playerRemoved, this, &NetworkClient::onPlayerRemoved);
        for (int i=0; i<game->count(); ++i) {
            Player * player = game->get(i);
            //onPlayerAdded(player, false);  // this sends a message, which we don't want atm
            connect(player, &Player::nameChanged, this, &NetworkClient::onPlayerNameChanged);
            connect(player, &Player::colorChanged, this, &NetworkClient::onPlayerColorChanged);
            connect(player, &Player::scoreChanged, this, &NetworkClient::onPlayerScoreChanged);
        }
        for (QObject * catObj : game->board()->categories()) {
            Category * cat = qobject_cast<Category*>(catObj);
            for (QObject * questionObj : cat->questions()) {
                Question * question = qobject_cast<Question*>(questionObj);
                connect(question, &Question::revealedChanged, this, &NetworkClient::onQuestionRevealedChanged);
            }
        }
		m_player = nullptr;
	}

	void send(const QString & text) {
		m_socket->sendTextMessage(text);
	}

    void close() {
        send("end");
        m_socket->close();
    }

	QString connectionString() const {
		return QString("%1:%2").arg(m_socket->peerAddress().toString()).arg(m_socket->peerPort());
	}

	Game * game() const {
		return m_game;
	}

	Player * player() const {
		return m_player;
	}

	private slots:
	void onTextMessageReceived(const QString & message) {
		QStringList args = message.trimmed().split(" ");
		QString cmd = args.at(0);
		args.pop_front();

		if(cmd == "hello") {
			onCommandHello(std::move(args));
		}
		else if(cmd == "auth") {
			onCommandAuth(std::move(args));
		}
		else if(cmd == "color") {
			onCommandColor(std::move(args));
		}
		else if(cmd == "buzz") {
			onCommandBuzz(std::move(args));
		}
		else if(cmd == "nextColor") {
			onCommandNextColor(std::move(args));
		}
		else if(cmd == "select") {
			onCommandSelect(std::move(args));
		}
		else if (cmd == "audience") {
			onCommandAudience(std::move(args));
		}
		else {
			send("error unrecognized command, try 'hello', 'color', or 'buzz'");
		}
	}

	void onQuestion(Question * question) {
        sendQuestion(question);
	}

	void onResolve(PlayerAnswer * answer) {
		Question * question = answer->question();
		Category * category = question->category();
		Player * player = answer->player();
		int categoryId = m_game->getCategoryId(category);
		int tileId = category->getTileId(question);
		int answerId = question->getAnswerId(answer);
		int playerId = player ? player->id() : -1;
        send(QStringLiteral("answer %1 %2 %3 %4 %5").arg(categoryId).arg(tileId).arg(answerId).arg(answer->score()).arg(playerId));
	}

	void onBuzzer(Player * player) {
		if (player != nullptr) {
            send(QStringLiteral("buzzer closed %1").arg(player->id()));
		}
        else if (m_game->openQuestion()) {
            send(QStringLiteral("buzzer open"));
        }
	}

    void onQuestionRevealedChanged(bool revealed) {
        Question * question = qobject_cast<Question*>(sender());
        Category * category = question->category();
        int categoryId = m_game->getCategoryId(category);
        int tileId = category->getTileId(question);
        send(QStringLiteral("tile %1 %2 %3 %4").arg(categoryId).arg(tileId).arg(question->points()).arg(question->revealed() ? "revealed" : "hidden"));
    }

	void onTurn(Player * player) {
		if (player != nullptr) {
            send(QStringLiteral("turn %2").arg(player->id()));
		}
	}

	void onDisconnect() {
		if (m_player) m_player->setActive(false);
		emit disconnected();
	}

	void onPlayerAdded(Player * player) {
		connect(player, &Player::nameChanged, this, &NetworkClient::onPlayerNameChanged);
		connect(player, &Player::colorChanged, this, &NetworkClient::onPlayerColorChanged);
		connect(player, &Player::scoreChanged, this, &NetworkClient::onPlayerScoreChanged);
		sendPlayer(player);
	}

	void onPlayerRemoved(Player * player) {
		disconnect(player, &Player::nameChanged, this, &NetworkClient::onPlayerNameChanged);
		disconnect(player, &Player::colorChanged, this, &NetworkClient::onPlayerColorChanged);
		disconnect(player, &Player::scoreChanged, this, &NetworkClient::onPlayerScoreChanged);
		send(QStringLiteral("leave %1").arg(player->id()));
	}

	void onPlayerNameChanged(QString const& name) {
		Player * player = qobject_cast<Player*>(sender());
		sendPlayer(player);
	}

	void onPlayerColorChanged(QColor const& color) {
		Player * player = qobject_cast<Player*>(sender());
		sendPlayer(player);
	}

	void onPlayerScoreChanged(int score) {
		Player * player = qobject_cast<Player*>(sender());
		sendPlayer(player);
	}

	private:
	void onCommandHello(QStringList && args) {
		QString name = args.join(" ");
		if(name.isEmpty()) {
			name = m_game->nextFreePlayerName();
		}

		bool needWelcome = true;
		Player * existingPlayer = m_game->getByName(name);
		if(m_player != nullptr && existingPlayer == m_player) {
			send("ok that's already you");
			return;
		}
		else if(existingPlayer != nullptr) {  // target is another player
			if(existingPlayer->active()) {
				send("error that player is already controlled by another client");
				return;
			}

			if(existingPlayer->token() != "") {
				send("error that player is secured with an auth token, please use the auth command instead");
				return;
			}

			if(m_player) {
				qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is NO LONGER controlling" << m_player->name();
				m_player->setActive(false);
			}
			else {
				needWelcome = true;
			}

			setPlayer(existingPlayer);
			m_player->setActive(true);
			qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is now controlling" << m_player->name();
		}
		else if(m_player != nullptr) {  // already joined and not trying to switch -> a rename
			needWelcome = false;
			QString oldName = m_player->name();
			m_player->setName(name);
			qCDebug(logNetworkPlayers) << "Player" << oldName << "is now" << name;
		}
		else {  // a new player
			Player * newPlayer = m_game->addPlayer(m_game->nextFreePlayerId(), name, m_game->nextFreePlayerColor());
			if(newPlayer) {
				setPlayer(newPlayer);
				m_player->setActive(true);
				qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is now controlling" << m_player->name();
			}
			else {
				send("error you cannot join a running game");
				return;
			}
		}

		if (needWelcome) {
			sendWelcome();
		}
	}

	void onCommandAuth(QStringList && args) {
		QString token = args.join(" ");
		if(token.isEmpty()) {
			send("error invalid authentication token");
			m_socket->abort();
			return;
		}

		Player * player = m_game->getByToken(token);
		if(player == nullptr) {
			send("error invalid authentication token");
			m_socket->abort();
			return;
		}

		if(m_player) {
			qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is NO LONGER controlling" << m_player->name();
			m_player->setActive(false);
		}

		bool needsWelcome = m_player == nullptr;
		setPlayer(player);
		m_player->setActive(true);
		qCDebug(logNetworkPlayers) << "Connection" << connectionString() << "is now controlling" << m_player->name();
		if(needsWelcome) {
			sendWelcome();
		}
	}

	void onCommandBuzz(QStringList && args) {
		if(args.size() != 0) {
			send("error invalid arguments, expected no arguments");
			return;
		}

		if(!m_player) {
			send("error you're not registered yet, send a 'hello' command first");
			return;
		}

		if(m_game->joinable()) {
			send("error you can't buzz now, the game hasn't started!");
			return;
		}

		if(m_game->openQuestion() == nullptr) {
			send("error you can't buzz now, there's no open question!");
			return;
		}

		if(m_game->buzzerPlayer() != nullptr) {
			send("error you can't buzz now, someone else has already buzzed: " + m_game->buzzerPlayer()->name());
			return;
		}

		bool gotIt = m_game->buzz(m_player);
		if(gotIt) {
			send("ok you buzzed!");
		}
		else {
			send("error buzzing failed");
		}
	}
	void onCommandColor(QStringList && args) {
		if(args.length() != 3) {
			send("error invalid arguments, expected `color <red:decimal> <green:decimal> <blue:decimal>` with rgb values between 0 and 255");
			return;
		}

		bool rok, gok, bok;
		int red = args[0].toInt(&rok);
		int green = args[1].toInt(&gok);
		int blue = args[2].toInt(&bok);
		if(!rok || !gok || !bok || red < 0 || green < 0 || blue < 0 || red > 255 || green > 255 || blue > 255) {
			send("error incorrect syntax; 'color red green blue' with rgb values between 0 and 255");
			return;
		}
		if(!m_player) {
			send("error you're not registered yet, send a 'hello' command first");
			return;
		}
		m_player->setColor(QColor(red, green, blue));
		send("ok your color was changed successfully");
	}

	void onCommandNextColor(QStringList && args) {
		if(args.size() != 0) {
			send("error invalid arguments, expected no arguments");
			return;
		}

		if(!m_player) {
			send("error you're not registered yet, send a 'hello' command first");
			return;
		}

		m_player->setColor(m_game->nextFreePlayerColor());
		send("ok your color was changed!");
	}

	void onCommandSelect(QStringList && args) {
        send("error not implemented");
	}

	void onCommandAudience(QStringList && args) {
		sendWelcome();
	}

	void sendPlayer(Player * player) {
		send(QStringLiteral("player %1 %2 %3 %4 %5 %6").arg(player->id()).arg(player->color().red()).arg(player->color().green()).arg(player->color().blue()).arg(player->score()).arg(player->name()));
	}

	void sendWelcome() {
		qInfo() << "welcoming Player";
		for (int i=0; i<m_game->count(); ++i) {
			Player * player = m_game->get(i);
			sendPlayer(player);
		}
		int category_index = 0;
		for(QObject * category_obj : m_game->board()->categories()) {
			Category * category = qobject_cast<Category *>(category_obj);
			send(QString("category %1 %2").arg(category_index).arg(category->label()));
			int question_index = 0;
			for(QObject * question_obj : category->questions()) {
				Question * question = qobject_cast<Question *>(question_obj);
				QString question_state = question->revealed() ? "revealed" : "hidden";
				send(QString("tile %1 %2 %3 %4").arg(category_index).arg(question_index).arg(question->points()).arg(question_state));
				int answer_index = 0;
				for(QObject * answer_obj : question->playerAnswers()) {
					PlayerAnswer * answer = qobject_cast<PlayerAnswer *>(answer_obj);
                    int playerId = answer->player() ? answer->player()->id() : -1;
                    send(QString("answer %1 %2 %3 %4 %5").arg(category_index).arg(question_index).arg(answer_index).arg(answer->score()).arg(playerId));
					answer_index++;
				}
				question_index++;
			}
			category_index++;
		}
		send("welcome");

        if (m_game->openQuestion() != nullptr) {
            sendQuestion(m_game->openQuestion());
            if (m_game->buzzerPlayer() != nullptr) {
                sendBuzzer(m_game->buzzerPlayer());
            }
        }
	}

    void sendQuestion(Question * question) {
        if(question == nullptr) {
            send("question closed");
        }
        else {
            if(question->question().startsWith("[img]")) {
                send("question open <image>");
            }
            else {
                send(QStringLiteral("question open %1").arg(question->question()));
                send("buzzer open");
            }
        }
    }

    void sendBuzzer(Player * player) {
        send(QStringLiteral("buzzer closed %1").arg(player->id()));
    }

	signals:
	void playerChanged(Player * player);
	void gameChanged(Game * game);
	void disconnected();

	public:
	QWebSocket * m_socket = nullptr;
	Game * m_game = nullptr;
	Player * m_player = nullptr;

	public slots:
	void setGame(Game * game) {
		if(m_game == game)
			return;

		m_game = game;
		emit gameChanged(m_game);
	}

	void setPlayer(Player * player) {
		if(m_player == player) return;

		m_player = player;
		emit playerChanged(m_player);
	}
};

class NetworkPlayers::Private : public QObject {
	Q_OBJECT

	public:
	Private(NetworkPlayers * q)
		: QObject(q), q(q), server("jeopardy", QWebSocketServer::SecureMode, this) {
		connect(&server, &QWebSocketServer::newConnection, this, &NetworkPlayers::Private::onClientConnected);
		connect(&server, &QWebSocketServer::sslErrors, this, &NetworkPlayers::Private::onSslErrors);
	}

    ~Private() {
        for (NetworkClient * client : clients) {
            client->close();
        }
    }

	NetworkPlayers * q;

	Game * game;

	// Properties
	QWebSocketServer server;
	QList<NetworkClient *> clients;

	NetworkClient * getController(Player * player);
	public slots:
	void onClientConnected();
	void onSslErrors(QList<QSslError> const& errors);
	void onClientPlayerChanged(Player * player);
	void onClientDisconnected();
};

NetworkClient * NetworkPlayers::Private::getController(Player * player) {
	auto it = std::find_if(clients.cbegin(), clients.cend(), [&player](auto client) { return client->m_player == player; });
	if(it == clients.cend()) return nullptr;
	return *it;
}

void NetworkPlayers::Private::onClientConnected() {
	QWebSocket * socket = server.nextPendingConnection();

	NetworkClient * client = new NetworkClient(game, socket, this);
	connect(client, &NetworkClient::playerChanged, this, &NetworkPlayers::Private::onClientPlayerChanged);
	connect(client, &NetworkClient::disconnected, this, &NetworkPlayers::Private::onClientDisconnected);
	qInfo(logNetworkPlayers) << "New connection from" << client->connectionString();

	clients.append(client);
}

void NetworkPlayers::Private::onClientPlayerChanged(Player * player) {
}

void NetworkPlayers::Private::onClientDisconnected() {
	NetworkClient * client = qobject_cast<NetworkClient *>(sender());
	clients.removeOne(client);
	qInfo(logNetworkPlayers) << "Lost connection from" << client->connectionString();
	delete client;
}

void NetworkPlayers::Private::onSslErrors(QList<QSslError> const& errors) {
	qCritical() << "SSL Errors";
}

NetworkPlayers::NetworkPlayers(QObject * parent)
	: QObject(parent), d(new Private(this)) {
}

NetworkPlayers::~NetworkPlayers() {
	//delete d; //parent relation does this
}

void NetworkPlayers::classBegin() {
	QSslConfiguration sslConfiguration;
	QFile sslCrtFile(QStringLiteral("../queopardy.crt.pem"));
	QFile sslKeyFile(QStringLiteral("../queopardy.key.pem"));
	sslCrtFile.open(QIODevice::ReadOnly);
	sslKeyFile.open(QIODevice::ReadOnly);
	QSslCertificate sslCrt(&sslCrtFile, QSsl::Pem);
	QSslKey sslKey(&sslKeyFile, QSsl::Rsa, QSsl::Pem);
	sslCrtFile.close();
	sslKeyFile.close();
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	sslConfiguration.setLocalCertificate(sslCrt);
	sslConfiguration.setPrivateKey(sslKey);
	d->server.setSslConfiguration(sslConfiguration);

	if(!d->server.listen(QHostAddress::Any, 31337)) {
		qCCritical(logNetworkPlayers) << "Unable to listen:" << d->server.errorString();
	}
	else {
		qInfo(logNetworkPlayers) << "Listening on" << d->server.serverUrl().toString();
	}
}

void NetworkPlayers::componentComplete() {
}

QString NetworkPlayers::listenAddress() const {
	return d->server.serverUrl().toString();
}

void NetworkPlayers::setGame(Game * game) {
	if(d->game == game)
		return;

	d->game = game;
	emit gameChanged(d->game);
}

Game * NetworkPlayers::game() const {
	return d->game;
}

#include "networkplayers.moc"
