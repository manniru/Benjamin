import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    color: "black"
    property string line_buf: ""

    Flickable
    {
        id: con_flickable
        anchors.fill: parent

        contentHeight: console_text.height+200
        contentWidth: childrenRect.width
        clip: true

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

        var pos_y = console_text.height - parent.height + 250

        if( pos_y<0 )
        {
            pos_y = 0
        }
        con_flickable.contentY = pos_y
    }

}
