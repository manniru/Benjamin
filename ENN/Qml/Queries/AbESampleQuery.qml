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
    width: e_samples_text.width + 20
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    color: "#2e2e2e"

    signal acceptDialog()

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            e_samples_text.forceActiveFocus();
        }
    }

    Text
    {
        id: e_samples_text

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 17
        font.pixelSize: 20

        text: "Do you want to generate ENN samples?"
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
        id: yes_no_guide
        anchors.top: e_samples_text.bottom
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 16

        text: "(Press Y:generate)"
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
        anchors.bottomMargin: 10
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
        anchors.bottomMargin: 10
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
        close();
    }

    onVisibleChanged:
    {
        if( visible )
        {
            e_samples_text.forceActiveFocus();
        }
    }

    onActiveChanged:
    {
        e_samples_text.forceActiveFocus();
    }
}
