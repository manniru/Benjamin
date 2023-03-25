import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    height: 30
    color: "transparent"

    property int num_id: 0
    property string sample_text: ""
    property string font_awesome_label:    fontAwesomeSolid.name
    property string remove_char: "\uf1f8"
    property string pause_char:  "\uf04c"
    property string play_char:   "\uf04b"
    property bool line_hovered: false
    property bool icon_hovered: false
    property bool line_clicked: false
    property bool play_clicked: false
    property bool focused: false

    signal removeClicked(int index)
    signal lineClicked(int index)
    signal arrowClicked(int key, int index)

    Keys.onPressed:
    {
        if( line_clicked )
        {
            if( event.key===Qt.Key_Delete )
            {
                removeClicked(num_id);
            }
            else if( event.key===Qt.Key_Up
                    || event.key===Qt.Key_Down )
            {
                arrowClicked(event.key, num_id);
            }
        }

    }

    onFocusedChanged:
    {
        if( !focused )
        {
            line_clicked = false;
        }
        else
        {
            line_clicked = true;
            forceActiveFocus();
        }
    }

    Rectangle
    {
        id: linenum_bg
        width: 50
        height: parent.height
        anchors.top: parent.top
        anchors.left: parent.left

        color:
        {
            if( line_clicked )
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
            if( line_clicked )
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
            onClicked:
            {
                line_clicked = true;
                parent.parent.forceActiveFocus();
                lineClicked(num_id);
            }
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
        Accessible.name: "document"
        anchors.left: text_bg.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 3

        text: sample_text
        font.pixelSize: 16
        color: "#c9c9c9"
    }

    Rectangle
    {
        id: remove_bg

        height: parent.height
        width: 45
        anchors.right: text_bg.right
        anchors.top: parent.top
        color:
        {
            if( line_clicked )
            {
                "#315265"
            }
            else if( line_hovered )
            {
                "#828282"
            }
            else
            {
                "transparent"
            }
        }

        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true
            onClicked:
            {
                removeClicked(num_id);
            }
            onEntered:
            {
                icon_hovered = true;
                line_hovered = true;
            }
            onExited:
            {
                icon_hovered = false;
                line_hovered = false;
            }
        }
    }

    Label
    {
        anchors.right: text_bg.right
        anchors.rightMargin: 15
        anchors.verticalCenter: text_area.verticalCenter
        horizontalAlignment: Text.AlignHCenter

        font.pixelSize: 16
        font.family: font_awesome_label
        text: remove_char
        color:
        {
            if( icon_hovered )
            {
                "#d9a74c"
            }
            else if( line_clicked || line_hovered )
            {
                "#9a9a9a"
            }
            else
            {
                "transparent"
            }
        }
    }

    Rectangle
    {
        height: parent.height
        width: 45
        anchors.right: remove_bg.left
        anchors.top: parent.top
        color:
        {
            if( line_clicked )
            {
                "#3e5765"
            }
            else if( line_hovered )
            {
                "#757575"
            }
            else
            {
                "transparent"
            }
        }
        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true
            onClicked:
            {
                play_clicked = !play_clicked;
            }
            onEntered:
            {
                icon_hovered = true;
                line_hovered = true;
            }
            onExited:
            {
                icon_hovered = false;
                line_hovered = false;
            }
        }
    }

    Label
    {
        anchors.right: remove_bg.left
        anchors.rightMargin: 15
        anchors.verticalCenter: text_area.verticalCenter
        horizontalAlignment: Text.AlignHCenter

        font.pixelSize: 16
        font.family: font_awesome_label
        text:
        {
            if( play_clicked )
            {
                pause_char;
            }
            else
            {
                play_char;
            }
        }
        color:
        {
            if( icon_hovered && play_clicked )
            {
                "#e1dfe2"
            }
            else if( icon_hovered && !play_clicked )
            {
                "#a5cca2"
            }
            else if( line_clicked || line_hovered )
            {
                "#9a9a9a"
            }
            else
            {
                "transparent"
            }
        }
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
}

