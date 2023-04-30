import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    height: linenum_lbl.height + 10
    property string word_shortcut: ""
    property string word_text: ""
    property string word_path: ""

    Rectangle
    {
        id: linenum_bg
        width: 50
        height: parent.height
        anchors.top: parent.top
        anchors.left: parent.left

        color: "#666666"
    }

    Label
    {
        id: linenum_lbl
        anchors.top: parent.top
        anchors.horizontalCenter: linenum_bg.horizontalCenter
        anchors.topMargin: 3

        font.pixelSize: 16
        text: word_shortcut
        color: "#b4b4b4"
    }

    Rectangle
    {
        id: text_bg
        height: parent.height
        anchors.top: parent.top
        anchors.left: linenum_bg.right
        width: parent.width - 20
        color: "#4e4e4e"
    }

    Label
    {
        id: phoneme_lbl
        anchors.left: linenum_lbl.right
        anchors.leftMargin: 36
        anchors.top: parent.top
        anchors.topMargin: 2

        text: word_text
        font.pixelSize: 16
        color: "#c9c9c9"
    }

}
