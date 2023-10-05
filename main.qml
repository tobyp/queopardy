import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import QtMultimedia 5.13
import QtGamepad 1.13
import com.queopardy 1.0

Window {
    id: app
    visible: true;
    width: 640;
    height: 480;
    title: qsTr("Queopardy");
    color: "lightgray"

//    visibility: Window.FullScreen
    flags: Qt.Window | Qt.WindowFullscreenButtonHint | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMinMaxButtonsHint |            Qt.WindowCloseButtonHint | Qt.WindowFullscreenButtonHint

    NetworkPlayers {
        id: netPlayers
        game: ctxGame
    }

    function configureGamepad(deviceId) {
        GamepadManager.configureButton(deviceId, GamepadManager.ButtonX)
        GamepadManager.configureButton(deviceId, GamepadManager.ButtonY)
        GamepadManager.configureButton(deviceId, GamepadManager.ButtonA)
        GamepadManager.configureButton(deviceId, GamepadManager.ButtonB)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonR1)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonR2)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonL1)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonL2)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonDown)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonLeft)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonRight)
        // GamepadManager.configureButton(deviceId, GamepadManager.ButtonUp)
    }

    Connections {
        id: gamepadManager
        target: GamepadManager

        Component.onCompleted: {
            console.log("Initial connected gamepads:", GamepadManager.connectedGamepads);

            gamePadModel.clear();

            for(var i in GamepadManager.connectedGamepads) {
                var deviceId = GamepadManager.connectedGamepads[i];

                gamePadModel.append({"deviceId": deviceId});

                configureGamepad(deviceId)
            }
        }

        function onConnectedGamepadsChanged() {
            console.log("Connected gamepad changed:", GamepadManager.connectedGamepads);

            gamePadModel.clear();

            for(var i in GamepadManager.connectedGamepads) {
                var deviceId = GamepadManager.connectedGamepads[i];

                gamePadModel.append({"deviceId": deviceId});

                configureGamepad(deviceId)
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
                    var playerIdx = GamepadManager.connectedGamepads.indexOf(deviceId);
                    var player = ctxGame.get(playerIdx);
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

    Connections {
        target: ctxGame

        function playerSound(player) {
            if (player) {
                var idx = ctxGame.indexOf(player);

                playSound(idx);
            }
        }

        function onBuzzerPlayerChanged(buzzerPlayer) {
            playerSound(buzzerPlayer);
        }

        function onPlayerActiveChanged(player) {
            if (player.active) playerSound(player);
        }
    }

    function playSound(idx) {
        var child = soundRepeater.itemAt(idx)
        var player = child.player;
        player.play();
    }

    Repeater {
        id: soundRepeater
        model: ctxGame.count

        Item {
            Component.onCompleted: {
                //console.log("Media Player created for player", model.index)
            }

            property MediaPlayer player: MediaPlayer {
                source: "buzz%1.mp3".arg(1 + model.index)
            }
        }
    }

    MediaPlayer {
        id: failSound
        source: "wrong-answer.mp3"
    }

    StackView {
        id: root;

        anchors.fill: parent;

        Component {
            id: questionComponent;
            QuestionDisplay {
                id: questionDisplay;

                onFailed: {
                    failSound.play();
                }

                onComplete: {
                    ctxGame.openQuestion = null;
                    root.pop();
                }

                focus: true
                Keys.enabled: true
                Keys.onPressed: {
                    if (ctxGame.state !== Game.QUESTION) return;

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
                    ctxGame.openQuestion = question;
                }
            }
        }

        initialItem: Splash {
            game: ctxGame
            onReady: function() {
                root.push(boardComponent);
                ctxGame.joinable = false;
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
