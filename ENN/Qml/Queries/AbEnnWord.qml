import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Window
{   
    title: ""
    height: 160
    width: 300
    x: word_list.x + width/2
    y: (root.height - height) / 2
    color: "#2e2e2e"

    property int    dialog_width: width

    signal acceptDialog(string value)

    Text
    {
        id: get_value_top_label

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 17
        font.pixelSize: 20

        text: "Enter Word ID to Train ENN:"

        color: "#b4b4b4"
    }

    Rectangle
    {
        height: childrenRect.height
        width: childrenRect.width
        anchors.top: get_value_top_label.bottom
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter
        color: "transparent"

        Text
        {
            id: get_value_label
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 2
            font.pixelSize: 20
            text: "Word ID:"
            height: 40
            color: "#b4b4b4"
        }

        TextField
        {
            id: get_value_input
            anchors.left: get_value_label.right
            anchors.leftMargin: 10
            anchors.top: parent.top
            width: dialog_width * 0.6
            color: "white"
            background: Rectangle
            {
                anchors.fill: parent
                color: "#666666"
            }

            text: "Input text"
            font.pixelSize: 16

            focus: true

            Keys.onPressed:
            {
                if( event.key===Qt.Key_Enter ||
                        event.key===Qt.Key_Return )
                {
                    accept();
                }
                else if( event.key===Qt.Key_Escape )
                {
                    reject();
                }
            }
        }
    }

    AbButton
    {
        id: ok_button
        text: "Ok"
        width: parent.width/4 - 5
        anchors.left: parent.left
        anchors.leftMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

        onClick:
        {
            accept();
        }
    }

    AbButton
    {
        id: cancel_button
        text: "Cancel"
        width: parent.width/4 - 5
        anchors.right: parent.right
        anchors.rightMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

        onClick:
        {
            reject();
        }
    }

    function accept()
    {
        acceptDialog(get_value_input.text);
        close();
    }

    function reject()
    {
        close();
    }

    onVisibleChanged:
    {
        if( visible )
        {
            get_value_input.text = "";
            get_value_input.forceActiveFocus();
        }
    }
}
