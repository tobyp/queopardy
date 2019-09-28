import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import QtGamepad 1.13
import com.queopardy 1.0

Gamepad {
    signal buzzed
    
    function buzzGamepad(button) {
        if (button) {
            buzzed();
        }
    }

    property bool anyButton: buttonA || buttonB || buttonX || buttonY

    onButtonAChanged: buzzGamepad(buttonA)
    onButtonBChanged: buzzGamepad(buttonB)
    onButtonXChanged: buzzGamepad(buttonX)
    onButtonYChanged: buzzGamepad(buttonY)
}
