import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import QtGamepad 1.13
import com.queopardy 1.0

Window {
    visible: true;
    width: 640;
    height: 480;
    title: qsTr("Queopardy");
    color: "lightgray"

    NetworkPlayers {
        id: netPlayers
        game: ctxGame
    }

    Connections {
        id: gamepadManager
        target: GamepadManager

        onConnectedGamepadsChanged: {
            console.log("Connected gamepad changed:", GamepadManager.connectedGamepads);

            gamePadModel.clear();

            for(var i in GamepadManager.connectedGamepads) {
                var deviceId = GamepadManager.connectedGamepads[i];

                gamePadModel.append({"deviceId": deviceId});
            }
        }
    }

    Repeater {
        model: ListModel {
            id: gamePadModel
        }

        Item {
            GamepadBuzzer {
                deviceId: model.deviceId

                onBuzzed: {
                    var player = ctxGame.get(deviceId);
                    if (player) {
                        console.log("Name", name, "buzzered player", player.name);

                        ctxGame.buzz(player);
                    } else {
                        console.error("Buzzed for unknown player number", deviceId);
                    }
                }
            }
        }
    }

    StackView {
        id: root;

        anchors.fill: parent;

        Component {
            id: questionComponent;
            QuestionDisplay {
                id: questionDisplay;

                onComplete: {
                    ctxGame.setState(Game.UNBUZZABLE);
                    root.pop();
                }

                focus: true
                Keys.enabled: true
                Keys.onPressed: {
                    if (!ctxGame.state === Game.BUZZABLE) return;

                    var buzzerIndex = -1;
                    if (event.key === Qt.Key_A) {
                        buzzerIndex = 0;
                    }
                    else if (event.key === Qt.Key_B) {
                        buzzerIndex = 1;
                    }
                    else if (event.key === Qt.Key_C) {
                        buzzerIndex = 2;
                    }
                    else if (event.key === Qt.Key_D) {
                        buzzerIndex = 3;
                    }
                    else if (event.key === Qt.Key_E) {
                        buzzerIndex = 4;
                    }
                    else if (event.key === Qt.Key_F) {
                        buzzerIndex = 5;
                    }
                    else if (event.key === Qt.Key_G) {
                        buzzerIndex = 6;
                    }
                    else if (event.key === Qt.Key_H) {
                        buzzerIndex = 7;
                    }
                    else if (event.key === Qt.Key_Escape) {
                        root.pop();
                        event.accepted = true;
                    }

                    if (buzzerIndex > -1 && buzzerIndex < ctxGame.count) {
                        event.accepted = true;
                        var player = ctxGame.get(buzzerIndex);
                        ctxGame.buzz(player);
                    }
                }
            }
        }

        Component {
            id: boardComponent;
            BoardView {
                game: ctxGame

                onQuestionSelected: function(question) {
                    root.push(questionComponent, {question: question});
                    ctxGame.setState(Game.BUZZABLE);
                }
            }
        }

        initialItem: Splash {
            game: ctxGame
            onReady: function() {
                root.push(boardComponent);
                ctxGame.setState(Game.UNBUZZABLE);
            }
        }

        pushEnter: Transition {
            ParallelAnimation {
                OpacityAnimator {
                    from: 0;
                    to: 1;
                    duration: 200;
                    easing.type: Easing.OutQuad;
                }
                ScaleAnimator {
                    from: 0;
                    to: 1;
                    duration: 200;
                    easing.type: Easing.OutQuad;
                }
            }
        }
        pushExit: Transition { }
        popEnter: Transition { }
        popExit: Transition {
            ParallelAnimation {
                OpacityAnimator {
                    from: 1;
                    to: 0;
                    duration: 200;
                    easing.type: Easing.OutQuad;
                }
            }
        }
    }
}
