import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Window
{
    title: "Delete File ?"
    height: changed_word_list.height + 150
    width: Math.max(changed_word_list.width, accept_label.width) + 50
    property string dialog_result: ""
    property var dialog_label: []
    property string botton_text: "#b6b6b6"
    property string botton_bg: "#4d4d4d"
    property string botton_border: "#bfbfbf"
    property string area_text: "#c9c9c9"
    property string font_name_label:    fontRobotoRegular.name

    color: "#2e2e2e"

    Text
    {
        id: accept_label
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.left: parent.left
        anchors.leftMargin: 20
        font.family: font_name_label
        font.pixelSize: 20
        text: "Are you sure to change these words?"
        lineHeight: 1.4
        color: area_text

        Keys.onPressed:
        {
            if( event.key===Qt.Key_Space ||
                event.key===Qt.Key_Y )
            {
                accept();
            }
            else if( event.key===Qt.Key_Q ||
                    event.key===Qt.Key_N )
            {
                reject();
            }
        }
    }

    GridLayout
    {
        id: changed_word_list
        rows: 10
        anchors.left: parent.left
        anchors.top: accept_label.bottom
        anchors.leftMargin: 30
        columnSpacing: 20
        flow: GridLayout.TopToBottom

        Repeater
        {
            model: dialog_label
            Text
            {
                text: modelData
                color: area_text
                font.family: font_name_label
                font.pixelSize: 20
            }
        }
    }

    AbButton
    {
        id: yes_button
        text: "Yes"
        width: 150
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 25
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

        onClick:
        {
            accept();
        }
    }

    AbButton
    {
        id: no_button
        text: "No"
        width: 150
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 25
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

        onClick:
        {
            reject();
        }
    }


    Component.onCompleted:
    {
        accept_label.forceActiveFocus();
    }

    function accept()
    {
        dialog_result = "Y";
        close();
    }

    function reject()
    {
        dialog_result = "N";
        close();
    }


}
