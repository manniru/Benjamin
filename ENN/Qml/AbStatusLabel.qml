import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    width: status_label.width + status_val_label.width
    height: status_label.height
    color: "transparent"

    property string font_name_label:    fontRobotoRegular.name
    property int    font_size:          24
    property int    status_state:       0
    property color  color_text:         "#b2b2b2"

    Text
    {
        id: status_label

        text: "Status: "
        anchors.left: parent.left
        anchors.top: parent.top
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label
        lineHeight: 1.2
    }

    Text
    {
        id: status_val_label

        text:
        {
            if( status_state===ab_const.ab_STATUS_REC )
            {
                "Rec"
            }
            else if( status_state===ab_const.ab_STATUS_PAUSE )
            {
                "Pause"
            }
            else if( status_state===ab_const.ab_STATUS_STOP )
            {
                "Stop"
            }
            else if( status_state===ab_const.ab_STATUS_REQPAUSE )
            {
                "Req Pause"
            }
            else if( status_state===ab_const.ab_STATUS_BREAK )
            {
                "Break"
            }
            else if( status_state===ab_const.ab_STATUS_PLAY )
            {
                "Play"
            }
            else if( status_state===ab_const.ab_STATUS_DECPAUESE )
            {
                "Decide Pause"
            }
        }
        anchors.left: status_label.right
        anchors.top: parent.top
        color:
        {
            if( status_state===ab_const.ab_STATUS_REC )
            {
                "#f00"
            }
            else if( status_state===ab_const.ab_STATUS_PAUSE )
            {
                "#b17400"
            }
            else if( status_state===ab_const.ab_STATUS_STOP )
            {
                "#10b100"
            }
            else if( status_state===ab_const.ab_STATUS_REQPAUSE )
            {
                "#008eca"
            }
            else if( status_state===ab_const.ab_STATUS_BREAK )
            {
                "#00b8d7"
            }
            else if( status_state===ab_const.ab_STATUS_PLAY )
            {
                "#f00"
            }
            else if( status_state===ab_const.ab_STATUS_DECPAUESE )
            {
                "#b17400"
            }
        }
        font.pixelSize: font_size
        font.family: font_name_label
        lineHeight: 1.2
    }
}
