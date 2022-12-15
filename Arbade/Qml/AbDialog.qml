import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3

Dialog
{
    title: ""
    height: 160
    width: 300
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    focus: true
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    property string dialog_text: ""

    Text
    {
        id: get_value_label
        anchors.left: parent.Left
        anchors.top: parent.top
        anchors.topMargin: 5
        font.pixelSize: 20
        text: "value"
        height: 40
    }
    TextField
    {
        id: get_value_input
        anchors.left: get_value_label.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        text: "Input text"
        font.pixelSize: 16
        width: parent.width * 0.75
        focus: true
        onFocusChanged: console.log("Focus changed " + focus)
        Keys.onReturnPressed: get_value_dialog.accept()
    }

    onAccepted:
    {
        console.log("Accepted")
        dialog_text = get_value_input.text
    }

    onRejected:
    {
        console.log("Rejected")
    }

    onOpened:
    {
        get_value_input.text = ""
    }
}
