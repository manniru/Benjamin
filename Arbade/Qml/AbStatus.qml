import QtQuick 2.0

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          24
    property color  color_text:         "#b2b2b2"

    property real   pause_time
    property real   num_words
    property real   rec_time
    property int    status
    property string words
    property string category
    property int    count
    property int    count_total
    property real   elapsed_time
    property real   power
    property string focus_word
    property string mean
    property string variance

    color: "transparent"

    Rectangle
    {
        width: 400
        height: childrenRect.height
        anchors.top: parent.top
        anchors.topMargin: 40

        color: "transparent"
//        color: "red"

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
                    "Req Pause"
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
                    "#008eca"
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
            anchors.leftMargin: 76
            anchors.top: count_label.bottom
            anchors.topMargin: 2
            color: color_text
            width: (elapsed_time/100)*178
            height: 25
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

        Text
        {
            id: mean_label

            text: "Mean: " + mean
            anchors.left: parent.left
            anchors.top: focus_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: var_label

            text: "Var: " + variance
            anchors.left: parent.left
            anchors.top: mean_label.bottom
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        AbButton
        {
            id: save_button
            text: "Save"
            width: .7*parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: var_label.bottom
            anchors.topMargin: 30
            enabled: false
        }

        AbButton
        {
            id: reset_button
            text: "Reset"
            width: .7*parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: save_button.bottom
            anchors.topMargin: 15
            enabled: false
        }
    }

}
