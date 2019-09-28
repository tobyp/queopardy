import QtQuick 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import "Colors.js" as C

Flipable {
    id: flipable;

    Layout.fillWidth: true;
    Layout.fillHeight: true;

    property var question;

    signal selected();

    transform: Rotation {
        id: rotation
        origin.x: flipable.width/2
        origin.y: flipable.height/2
        axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
        angle: 0    // the default angle
    }

    states: State {
        name: "back"
        PropertyChanges {
            target: rotation; angle: 180;
        }
        when: question.revealed;
    }

    transitions: Transition {
        NumberAnimation {
            target: rotation;
            property: "angle";
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    front: Button {
        anchors.fill: parent;

        text: question.points;
        font.pointSize: 30;
        onClicked: {
            selected();
        }
    }

    back: Rectangle {
        anchors.fill: parent;
        property var winner: question.playerAnswers.length > 0 ? question.playerAnswers[question.playerAnswers.length - 1].player : null;
        color: winner ? winner.color : "gray";
        id: tile

        MouseArea {
            anchors.fill: parent;
            onClicked: {
                if (mouse.modifiers & Qt.ControlModifier) {
                    selected();
                }
            }
        }
        ColumnLayout {
            anchors.centerIn: parent;

            Repeater {
                model: question.playerAnswers;

                delegate: Text {
                    Layout.alignment: Qt.AlignHCenter
                    property var playerAnswer: model.modelData;
                    color: C.textColorFor(tile.color)
                    text: {
                        if (playerAnswer.player !== null) {
                            return playerAnswer.player.name + " [" + playerAnswer.score + "]";
                        }
                        else {
                            return "Nobody";
                        }
                    }
                }
            }
        }
    }
}
