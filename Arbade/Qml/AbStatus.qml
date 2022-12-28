import QtQuick 2.0

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          28
    property color  color_text:         "#b2b2b2"

    property real   pause_time:         1
    property real   num_words:          3
    property real   rec_time:           3
    property int    status:             1
    property string words:              ""
    property string category:           "sag"
    property int    count:              5
    property int    count_total:        100
    property real   elapsed_time:       0
    property real   power:              -45
    property string focus_word:         ""

    color: "transparent"

    Rectangle
    {
        width: childrenRect.width
        height: childrenRect.height
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 20

        color: "transparent"

        Text
        {
            id: pause_label

            text: "Pause Time: " + pause_time.toFixed(1) + " sec"
            anchors.left: parent.left
            anchors.top: parent.top
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: num_words_label

            text: "Num of Words: " + num_words.toString()
            anchors.left: parent.left
            anchors.top: pause_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: rec_label

            text: "Rec Time: " + rec_time.toFixed(1) + " sec"
            anchors.left: parent.left
            anchors.top: num_words_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: status_label

            text: "Status: "
            anchors.left: parent.left
            anchors.top: rec_label.bottom
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
                if( status===ab_const.ab_STATUS_REC )
                {
                    "Rec"
                }
                else if( status===ab_const.ab_STATUS_PAUSE )
                {
                    "Pause"
                }
                else if( status===ab_const.ab_STATUS_STOP )
                {
                    "Stop"
                }
                else if( status===ab_const.ab_STATUS_REQPAUSE )
                {
                    "Rec"
                }
                else if( status===ab_const.ab_STATUS_BREAK )
                {
                    "Break"
                }
                else if( status===ab_const.ab_STATUS_PLAY )
                {
                    "Play"
                }
            }
            anchors.left: status_label.right
            anchors.top: rec_label.bottom
            color:
            {
                if( status===ab_const.ab_STATUS_REC )
                {
                    "#f00"
                }
                else if( status===ab_const.ab_STATUS_PAUSE )
                {
                    "#b17400"
                }
                else if( status===ab_const.ab_STATUS_STOP )
                {
                    "#10b100"
                }
                else if( status===ab_const.ab_STATUS_REQPAUSE )
                {
                    "#f00"
                }
                else if( status===ab_const.ab_STATUS_BREAK )
                {
                    "#00b8d7"
                }
                else if( status===ab_const.ab_STATUS_PLAY )
                {
                    "#f00"
                }
            }
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: words_label

            text: "Word: " + words
            anchors.left: parent.left
            anchors.top: status_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: category_label

            text: "Category: \"" + category + "\""
            anchors.left: parent.left
            anchors.top: words_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: count_label

            text: "Count: [" + count.toString() + "/" +
                  count_total.toString() + "]"
            anchors.left: parent.left
            anchors.top: category_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: time_label

            text: "Time: [                               ]"
            anchors.left: parent.left
            anchors.top: count_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Rectangle
        {
            anchors.left: time_label.left
            anchors.leftMargin: 90
            anchors.top: count_label.bottom
            anchors.topMargin: 3
            color: color_text
            width: (elapsed_time/100)*206
            height: 30
        }

        Text
        {
            id: power_label

            text: "Power: " + power.toFixed(2).toString() + "dB"
            anchors.left: parent.left
            anchors.top: time_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: focus_label

            text: "Focus word: " + focus_word
            anchors.left: parent.left
            anchors.top: power_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }
    }

}
