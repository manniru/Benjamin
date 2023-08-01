import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property real   pause_time
    property real   num_words
    property real   rec_time
    property int    count_total
    property string focus_word
    property string font_name_label:    fontRobotoRegular.name
    property int    font_size:          24
    property color  color_text:         "#b2b2b2"

    color: "#262626"

    GridLayout
    {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.topMargin: 10

        rows: 1
        columns: 5
        flow: GridLayout.TopToBottom
//        columnSpacing: 50
        rowSpacing: -10

        Text
        {
            id: pause_label

            text: "Pause Time: " + pause_time.toFixed(1) + " sec"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: num_words_label

            text: "Num of Words: " + num_words.toString()
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: rec_label

            text: "Rec Time: " + rec_time.toFixed(1) + " sec"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: count_label

            text: "Count: " + count_total.toString()
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: focus_label

            text: "Focus word: " + focus_word
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }
    }

}
