import QtQuick 2.13
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5
import QtMultimedia 5.13

Rectangle {
    id: qDisplay;

    signal complete();
    color: "lightgray"
    property var question: null;

    Connections {
        target: ctxGame
        onPlayerBuzzed: {
            if (buzzer.player !== null) return;  //only buzz when open
            buzzer.player = player;
        }
    }

    focus:true
    Keys.enabled: true
    Keys.onPressed: {
        if (event.key === Qt.Key_Delete) {
            question.revealed = true;
            qDisplay.question.addPlayerAnswer(null, 0);
            buzzer.player = null;
            qDisplay.complete();
        }
    }

    MediaPlayer {
        id: playMusic
        source: "think.mp3"
        autoPlay: true
    }

    Rectangle {  //buzzer screen
        anchors.fill: parent;
        color: Qt.rgba(0, 0, 0, 0.5);
        z: 100;
        visible: buzzer.player !== null;

        onVisibleChanged: {
            if (visible) {
                playMusic.pause();
            } else {
                playMusic.play();
            }
        }

        GridLayout {
            anchors.fill: parent;
            Rectangle {
                Layout.alignment: Qt.AlignCenter
                id: buzzer;

                width: childrenRect.width;
                height: childrenRect.height;
                property var player: null;
                color: player !== null ? player.color : "transparent";

                ColumnLayout {
                    Text {
                        color: buzzer.player !== null ? Qt.darker(buzzer.player.color, 3.0) : "black";
                        Layout.margins: 10;
                        Layout.alignment: Qt.AlignHCenter
                        font.pointSize: 20;
                        text: buzzer.player !== null ? buzzer.player.name : "";
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        Layout.margins: 10;
                        Button {
                            text: "Correct"
                            onClicked: {
                                question.revealed = true;
                                qDisplay.question.addPlayerAnswer(buzzer.player, qDisplay.question.points);
                                buzzer.player = null;
                                qDisplay.complete();
                            }
                        }
                        Button {
                            text: "Incorrect"
                            onClicked: {
                                qDisplay.question.addPlayerAnswer(buzzer.player, -qDisplay.question.points);
                                buzzer.player = null;
                            }
                        }
                        Button {
                            text: "Fail"
                            onClicked: {
                                question.revealed = true;
                                qDisplay.question.addPlayerAnswer(null, 0);
                                buzzer.player = null;
                                qDisplay.complete();
                            }
                        }
                        Button {
                            text: "Unbuzz"
                            onClicked: {
                                buzzer.player = null;
                            }
                        }
                        Button {
                            text: "Unclick"
                            onClicked: {
                                buzzer.player = null;
                                qDisplay.complete();
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: compText;
        GridLayout {
            FontLoader {
                id: emojiFont;
                source: question.font ? resourceUrl + "/" + question.font : "";
                onNameChanged: {
                    textText.font.family = name;
                }
            }
            Text {
                id: textText;
                font.pointSize: question.fontSize;
                wrapMode: Text.Wrap;
                text: textString;
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignCenter;
                Layout.margins: 10;
                Layout.fillHeight: true;
                Layout.fillWidth: true;
                //font.family: question.font ? emojiFont.name : "";
            }
        }
    }


    Component {
        id: compImg;
        GridLayout {
            Image {
                Layout.alignment: Qt.AlignCenter;
                Layout.fillHeight: true;
                Layout.fillWidth: true;
                Layout.margins: 10;
                source: imgUrl;
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    Loader {
        anchors.fill: parent;
        property var question;
        property url imgUrl;
        property var textString;
        id: loader
    }

    onQuestionChanged: {
        if (!question) {
            loader.sourceComponent = null;
            return;
        }
        else {
            loader.question = question;
            if (question.question.indexOf("[img]") === 0) {
                loader.imgUrl = Qt.resolvedUrl(resourceUrl + "/" + question.question.substring(5));
                loader.sourceComponent = compImg;
            }
            else {
                loader.textString = question.question;
                loader.sourceComponent = compText;
            }
        }
    }
}
