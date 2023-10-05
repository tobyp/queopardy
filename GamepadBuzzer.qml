import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.5
import QtGamepad 1.13
import com.queopardy 1.0

Gamepad {
    signal buzzed

    function buzzGamepad(button, state) {
        // console.log("Button", button, state)
        if (state) {
            buzzed();
        }
    }

    property bool anyButton: buttonA || buttonB || buttonX || buttonY  // || buttonL1 || buttonL2 || buttonR1 || buttonR2

    onButtonAChanged: buzzGamepad("A", buttonA)
    onButtonBChanged: buzzGamepad("B", buttonB)
    onButtonXChanged: buzzGamepad("X", buttonX)
    onButtonYChanged: buzzGamepad("Y", buttonY)
    // onButtonL1Changed: buzzGamepad("L1", buttonL1)
    // onButtonL2Changed: buzzGamepad("L2", buttonL2)
    // onButtonR1Changed: buzzGamepad("R1", buttonR1)
    // onButtonR2Changed: buzzGamepad("R2", buttonR2)
}
