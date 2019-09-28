import QtQuick 2.13
import QtQuick.Layouts 1.11
import com.queopardy 1.0

ColumnLayout {
    id: board;

    property Game game;

    signal questionSelected(var question);
    spacing: 2;

    RowLayout {
        Layout.fillWidth: true;
        Layout.fillHeight: true;

        spacing: 2;

        Repeater {
            id: categoryRepeater
            model: game.board.categories;

            delegate: ColumnLayout {
                Layout.fillWidth: true;
                spacing: 2;

                property var category: model.modelData;

                Rectangle {  // Category header
                    height: txt.height;
                    Layout.fillWidth: true;
                    color: "darkgray"
                    Text {
                        id: txt;
                        anchors.centerIn: parent;
                        text: category.label;
                        font.pointSize: 20;
                        color: "white";
                        padding: 10;
                    }
                }

                Repeater {
                    model: category.questions;

                    delegate: Tile {  // Question tile
                        id: tile;

                        question: model.modelData;

                        onSelected: {
                            board.questionSelected(question);
                        }
                    }
                }
            }
        }
    }

    PlayerBar {
        id: playerBar
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        model: game
    }
}

