import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10
import QtMultimedia 5.5

Rectangle
{
    height: 30
    color: "transparent"

    property int num_id: 0
    property string sample_text: ""
    property bool line_hovered: false
    property bool line_focused: word_list.focus_index===num_id

    Rectangle
    {
        id: linenum_bg
        width: 50
        height: parent.height
        anchors.top: parent.top
        anchors.left: parent.left

        color:
        {
            if( line_focused )
            {
                "#4e7084"
            }
            else if( line_hovered )
            {
                "#808080"
            }
            else
            {
                "#666666"
            }
        }
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
        height: parent.height
        width: parent.width - 50
        anchors.top: parent.top
        anchors.left: linenum_bg.right
        color:
        {
            if( line_focused )
            {
                "#475a65"
            }
            else if( line_hovered )
            {
                "#696969"
            }
            else
            {
                "#4e4e4e"
            }
        }

        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true

            onClicked: handleFocus()
            onEntered:
            {
                line_hovered = true;
            }
            onExited:
            {
                line_hovered = false;
            }
        }
    }

    Label
    {
        id: text_area

        anchors.left: text_bg.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 3

        text: sample_text
        textFormat: Text.PlainText
        font.pixelSize: 16
        color: "#c9c9c9"
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }

    function handleFocus()
    {
        if( word_list.focus_index===num_id )
        {
            word_list.focus_index = -1;
        }
        else
        {
            word_list.focus_index = num_id;
        }
    }
}

