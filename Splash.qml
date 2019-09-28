import QtQuick 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import com.queopardy 1.0;
import "Colors.js" as Colors;

Rectangle {
    id: root;

    color: "lightgray";

    property Game game;

    signal ready();

    function addPlayer() {
        if (txtPlayerName.text == "")
            return;

        root.game.addPlayer(txtPlayerName.text, colPlayerColor.color);

        txtPlayerName.clear();
        colPlayerColor.advanceColor();
    }

    GridLayout {
        anchors.fill: parent;

        Rectangle {
            Layout.alignment: Qt.AlignCenter;
            Layout.minimumWidth: 400;
            Layout.preferredHeight: childrenRect.height;

            ColumnLayout {
                anchors.fill: parent;
                Layout.alignment: Qt.AlignCenter
                spacing: 2;

                Text {
                    Layout.margins: 10;
                    Layout.fillWidth: true;
                    horizontalAlignment: Text.AlignHCenter;
                    text: "Players"
                }

                Repeater {
                    id: playerRepeater
                    model: ctxGame
                    delegate: RowLayout {
                        Layout.fillWidth: true;

                        Text {
                            id: playerText
                            Layout.fillWidth: true;
                            Layout.alignment: Qt.AlignVCenter;
                            text: model.name;

                            font.pointSize: playerBuzzer.anyButton ? 30 : 14

                            GamepadBuzzer {
                                id: playerBuzzer
                                deviceId: model.index
                            }
                        }

                        Rectangle {
                            width: btn.height;
                            height: btn.height;
                            color: model.color;
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    var player = ctxGame.get(model.index);

                                    player.color = Colors.nextColor(this.color);
                                }
                            }
                        }

                        Button {
                            id: btn;
                            text: "-"
                            onClicked: {
                                ctxGame.removePlayer(ctxGame.get(model.index));
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true;
                    TextField {
                        id: txtPlayerName;
                        Layout.fillWidth: true;
                        placeholderText: "Player Name"
                        onAccepted: root.addPlayer()
                    }

                    Rectangle {
                        id: colPlayerColor;
                        width: btn2.height;
                        height: btn2.height;
                        color: Colors.COLORS[0];

                        function advanceColor() {
                            this.color = Colors.nextColor(this.color);
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                colPlayerColor.advanceColor();
                            }
                        }
                    }

                    Button {
                        id: btn2;
                        text: "+"
                        onClicked: root.addPlayer()
                    }
                }

                Button {
                    Layout.fillWidth: true;
                    text: "Jeopardy!"
                    enabled: ctxGame.count >= ctxGame.minPlayers;
                    onClicked: {
                        root.ready();
                    }
                }
            }
        }
    }

}
