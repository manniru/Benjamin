import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

Rectangle
{
    color: "black"
    property string line_buf: ""

    Flickable
    {
        id: con_flickable
        anchors.fill: parent

        contentHeight: console_text.height+200
        contentWidth: parent.width
        clip: true

        ScrollBar.vertical: ScrollBar
        {
            width: 10
            anchors.right: parent.right // adjust the anchor as suggested by derM
        }

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
    }


    function addLine()
    {
        console_text.text += line_buf;

        var pos_y = console_text.height - parent.height + 200

        if( pos_y<0 )
        {
            pos_y = 0
        }
        con_flickable.contentY = pos_y
    }

}
