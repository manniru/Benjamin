import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    color: "#2e2e2e"

    property int    status
    property string words
    property real   elapsed_time
    property real   power
    property color  color_text:         "#b2b2b2"
    property int    font_size:          24
    property string font_name_label:    fontRobotoRegular.name

    GridLayout
    {
        anchors.centerIn: parent

        height: 200
        width:  600

        rows: 3
        columns: 2
        flow: GridLayout.TopToBottom
//        columnSpacing: 50
//        rowSpacing: -10

        Text
        {
            id: count_label

            text: if( root.ab_verifier )
                  {
                      "Count: [" + root.ab_count.toString() + "/" +
                        root.ab_total_count_v.toString() + "]"
                  }
                  else
                  {
                      "Count: [" + root.ab_count.toString() + "/" +
                        root.ab_total_count.toString() + "]"
                  }

            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: power_label

            text: "Power: " + power.toFixed(2).toString() + "dB"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: category_label

            text: if( root.ab_verifier===1 )
                  {
                      "Category: \"unverified\""
                  }
                  else if( root.ab_verifier===2 )
                  {
                      "Category: \"shit\""
                  }
                  else
                  {
                      "Category: \"" + editor_box.category + "\""
                  }
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
        }

        Text
        {
            id: words_label

            text: "Word: " + words
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
            lineHeight: 1.2
            textFormat: Text.PlainText
        }

        AbStatusLabel
        {
            status_state: status
        }

        AbTimeBar
        {
            time: elapsed_time
        }
    }

    signal acceptDialog(string value)
}
