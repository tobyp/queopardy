import QtQuick 2.13
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5
import QtMultimedia 5.13
import com.queopardy 1.0
import "Colors.js" as C

Rectangle {
    id: qDisplay;

    signal failed
    signal complete();
    color: "lightgray"
    property var question: null;

    Connections {
        target: ctxGame
    }

    function unbuzz(compl, fail) {
        ctxGame.buzzerPlayer = null;
        if (compl) {
            question.revealed = true;
            complete();
        }
        if (fail) {
            qDisplay.failed()
        }
    }

    focus:true
    Keys.enabled: true
    Keys.onPressed: {
        if (event.key === Qt.Key_Delete) {
            qDisplay.question.addPlayerAnswer(null, 0);
            unbuzz(true, true);
        }
        else if (event.key === Qt.Key_Escape) {
            unbuzz(ctxGame.buzzerPlayer === null);
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
        visible: ctxGame.buzzerPlayer !== null;

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
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.bottomMargin: 10
                id: buzzer;

                width: childrenRect.width;
                height: childrenRect.height;
                color: ctxGame.buzzerPlayer !== null ? ctxGame.buzzerPlayer.color : "transparent";

                ColumnLayout {
                    Text {
                        color: ctxGame.buzzerPlayer !== null ? C.textColorFor(ctxGame.buzzerPlayer.color) : "white";
                        Layout.margins: 10;
                        Layout.alignment: Qt.AlignHCenter
                        font.pointSize: 20;
                        text: ctxGame.buzzerPlayer !== null ? ctxGame.buzzerPlayer.name : "";
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        Layout.margins: 10;
                        Button {
                            text: "Correct"
                            onClicked: {
                                qDisplay.question.addPlayerAnswer(ctxGame.buzzerPlayer, qDisplay.question.points);
                                unbuzz(true);
                            }
                        }
                        Button {
                            text: "Incorrect"
                            onClicked: {
                                qDisplay.question.addPlayerAnswer(ctxGame.buzzerPlayer, -qDisplay.question.points);
                                unbuzz(false, true);
                            }
                        }
                        Button {
                            text: "Fail"
                            onClicked: {
                                qDisplay.question.addPlayerAnswer(null, 0);
                                unbuzz(true, true);
                            }
                        }
                        Button {
                            text: "Unbuzz"
                            onClicked: {
                                unbuzz(false);
                            }
                        }
                        Button {
                            text: "Unclick"
                            onClicked: {
                                unbuzz(true);
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
