import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    color: "black"
    property string line_buf: ""

    Text
    {
        id: console_text

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 5

        text: ""
        textFormat: Text.RichText
        color: "white"
        font.pixelSize: 14
    }

    function addLine()
    {
        console_text.text += line_buf;
    }

}
