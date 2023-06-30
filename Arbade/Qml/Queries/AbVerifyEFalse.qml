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
    height: 180
    width: efalse_query_text.width + 60
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    color: "#2e2e2e"

    property int efalse_num: 0

    signal acceptDialog()
    signal rejectDialog()

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            efalse_query_text.forceActiveFocus();
        }
    }

    Text
    {
        id: efalse_query_text

        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 17
        font.pixelSize: 20

        text: efalse_num + " files in " +
              "EFalse directory.\n" +
              "Do you want to verify?"

        color: "#b4b4b4"

        Keys.onPressed:
        {
            if( event.key===Qt.Key_Enter ||
                event.key===Qt.Key_Return ||
                event.key===Qt.Key_Y )
            {
                accept();
            }
            else if( event.key===Qt.Key_Escape ||
                     event.key===Qt.Key_N )
            {
                reject();
            }
        }
    }

    Text
    {
        anchors.top: efalse_query_text.bottom
        anchors.topMargin: 17
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 16

        text: "(Press Y:yes, N:copy to online)"
        color: "#b4b4b4"
    }

    AbButton
    {
        id: yes_button
        text: "Yes"
        width: parent.width/4 - 5
        anchors.left: parent.left
        anchors.leftMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
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
        width: parent.width/4 - 5
        anchors.right: parent.right
        anchors.rightMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

        onClick:
        {
            reject();
        }
    }

    function accept()
    {
        acceptDialog();
        close();
    }

    function reject()
    {
        rejectDialog();
        close();
    }

    onVisibleChanged:
    {
        if( visible )
        {
            efalse_query_text.forceActiveFocus();
        }
    }

    onActiveChanged:
    {
        efalse_query_text.forceActiveFocus();
    }
}
