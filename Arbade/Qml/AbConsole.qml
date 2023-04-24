import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

Rectangle
{
    color: "black"
    property string line_buf: ""
    property int    line_num: 1

    Rectangle
    {
        color: "#061B23"
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: console_linenum.width+15
    }

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
            anchors.topMargin: 5
            anchors.left: console_linenum.right
            anchors.leftMargin: 20

            text: "> "
            lineHeight: 1.1
//            textFormat: Text.RichText
            color: "#ccc"
            font.pixelSize: 15
        }

        Text
        {
            id: console_linenum

            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 8

            text: "0001\n"
            lineHeight: 1.1
            color: "#809098"
            font.pixelSize: 15
        }
    }

    function addLine()
    {
        line_num++;
        var zero = 4 - line_num.toString().length + 1;
        console_linenum.text += Array(+(zero>0 && zero)).join("0");
        console_linenum.text += line_num + "\n";
        console_text.text += line_buf;

        var pos_y = console_text.height - parent.height + 200

        if( pos_y<0 )
        {
            pos_y = 0
        }
        con_flickable.contentY = pos_y
    }

    function addText()
    {
        console_text.text += line_buf;
    }
}
