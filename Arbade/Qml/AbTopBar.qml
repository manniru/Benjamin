import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          20
    property color  color_text:         "#9a9a9a"

    property var help_text: ["Space: Rec/Pause","S: Set Category",
                             "Up/Down: Pause Time",
                             "Right/Left: Rec Time",
                             "A: Show LER", "C: Change Count",
                             "D: Gen E-Sample", "E: Train ENN",
                             "J/K: Word Num",
                             "O:Open Category", "T: Train",
                             "V:Verify Mode"]
    property real model_wer: 0 // word error rate
    property real model_ser: 0 // sentence error rate

    color: "#262626"
//    color: "yellow"

    GridLayout
    {
        rows: 2
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        columnSpacing: 20
        flow: GridLayout.TopToBottom

        Repeater
        {
            model: help_text
            Text
            {
                text: modelData
                color: color_text
                font.pixelSize: font_size
                font.family: font_name_label
            }
        }

        Text
        {
            text: "W:All Stat"
            color: if( root.ab_all_stat )
                   {
                       "#cd8968"
                   }
                   else
                   {
                       color_text
                   }
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Text
        {
            text: "/:Show Help"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Text
        {
            text: "Z:Delete Sample"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Text
        {
            text:
            {
                if( root.default_func_v===ab_const.ab_VMODE_WRONG )
                {
                    "R:Wrong Mode"
                }
                else if( root.default_func_v===ab_const.ab_VMODE_COPY )
                {
                    "R:Copy Mode"
                }
                else
                {
                    "R:Trash Mode"
                }
            }
            color:
            {
                if( root.default_func_v===ab_const.ab_VMODE_WRONG )
                {
                    "#cd8968"
                }
                else if( root.default_func_v===ab_const.ab_VMODE_COPY )
                {
                    color_text
                }
                else // verify trash mode
                {
                    "#cd68cb"
                }
            }
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Text
        {
            text: "WER: " + model_wer + "%"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Text
        {
            text: "SER:  " + model_ser + "%"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
        }
    }
}
