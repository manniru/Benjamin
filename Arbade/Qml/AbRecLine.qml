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
    property string font_awesome_label:    fontAwesomeSolid.name
    property string path: ""
    property bool line_hovered: false
    property bool line_focused: rec_list.focus_index===num_id

    signal removeClicked(int id)

    Keys.onPressed:
    {
        if( rec_list.focus_index===num_id )
        { 
            if( event.key===Qt.Key_Delete )
            {
                removeClicked(num_id);
            }
            else if( event.key===Qt.Key_Up )
            {
                var count = rec_list.getChildLen() - 1;

                if( rec_list.focus_index<count )
                {
                    rec_list.focus_index++;
                }
            }
            else if( event.key===Qt.Key_Down )
            {
                if( rec_list.focus_index>0 )
                {
                    rec_list.focus_index--;
                }
            }
            else if( event.key===Qt.Key_Space )
            {
                playAudio();
            }
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

    Rectangle
    {
        id: audio_bg
        height: parent.height
        width: (parent.width - 50) * (rec_player.position/rec_player.duration)
        anchors.top: parent.top
        anchors.left: linenum_bg.right
        color: "#406e78"
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

    AbRecDelete
    {
        id: delete_button

        height: parent.height
        width: 45
        anchors.right: text_bg.right
        anchors.top: parent.top

        line_hvrd: line_hovered
        line_fcsd: line_focused

        onHoverChange: line_hovered = h
        onClick: removeClicked(num_id);
    }

    AbRecPlay
    {
        id: play_button

        height: parent.height
        width: 45
        anchors.right: delete_button.left
        anchors.top: parent.top

        line_hvrd: line_hovered
        line_fcsd: line_focused

        onHoverChange: line_hovered = h
        onClick: playAudio()
    }

    Audio
    {
        id: rec_player
        source: path

        onStopped:
        {
            play_button.playing = false;
            audio_bg.visible = false;
        }
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }

    function handleFocus()
    {
        if( rec_list.focus_index===num_id )
        {
            rec_list.focus_index = -1;
        }
        else
        {
            rec_list.focus_index = num_id;
        }
    }

    function playAudio()
    {
        audio_bg.visible = true;
        rec_list.focus_index = num_id;
        play_button.playing = true;
        rec_player.play();
    }
}

