import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    id: wordline
    height: 30
    color: "transparent"

    property int num_id: 0
    property string text: "value"
    property string font_awesome_label:    fontAwesomeSolid.name

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
        text: zeroPad(num_id)
        color: "#b4b4b4"
    }

    Rectangle
    {
        id: text_bg
        height: text_area.height
        anchors.top: parent.top
        anchors.left: linenum_bg.right
        width: wordline.width - 20
        color: "#4e4e4e"
    }

    Label
    {
        id: text_area
        Accessible.name: "document"
        width: wordline.width - 70
        anchors.left: linenum_lbl.right
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: -2

        text: "word_text"
        font.pixelSize: 16
        focus: false
        color: "#c9c9c9"
    }

    Label
    {
        id: stat_lbl
        anchors.top: parent.top
        anchors.right: text_bg.right
        anchors.rightMargin: 20
        anchors.topMargin: 3
        horizontalAlignment: Text.AlignHCenter

        font.pixelSize: 16
        font.family: font_awesome_label
        text: "\uf04c"
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
}

