import QtQuick 2.13
import QtQuick.Layouts 1.11
import com.queopardy 1.0
import "Colors.js" as C

Flow {
    id: players;

    spacing: 10;

    property alias model: playerRepeater.model;

    Repeater {
        id: playerRepeater;
//        model: [];
        delegate: Rectangle {
            property int playerIndex: model.index;
            property Player player: players.model.get(playerIndex)

            Layout.preferredWidth: childrenRect.width;
            height: childrenRect.height;
            width: childrenRect.width;

            color: player.color;

            Text {
                text: player.name + ": " + player.score;
                color: {
                    return C.textColorFor(player.color);
                }
                font.pointSize: 20;
                padding: 5;
                leftPadding: 10;
                rightPadding: 10;
            }
        }
    }
}
