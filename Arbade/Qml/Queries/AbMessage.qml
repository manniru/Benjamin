import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3

Rectangle
{
    property string message: ""

    width: message_lbl.width + 100
    height: message_lbl.height + 30
    color: "#2e2e2e"
    visible: false
    radius: 20
    border.color: "#bbb"
    border.width: 2

    onMessageChanged: showMessage()

    Label
    {
        id: message_lbl
        anchors.centerIn: parent

        text: message
        font.pixelSize: 16
        color: "#c9c9c9"
    }

    Timer
    {
        id: msg_timer
        interval: 5000
        repeat: false

        onTriggered:
        {
            parent.visible = false;
            message = "";
        }
    }

    function showMessage()
    {
        if( message.length )
        {
            msg_timer.start();
            visible = true;
        }
    }
}
