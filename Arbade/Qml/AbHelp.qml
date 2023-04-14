import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          22
    property color  color_text:         "#9a9a9a"

    property var help_text: ["Space: Pause Recording","S: Set Category",
                             "Up/Down: Change Pause",
                             "Right/Left: Change Rec Time",
                             "C: Change Count","F: Focus Word",
                             "J: Decrease Word", "K: Increase Word",
                             "O:Open Category", "T: Train",
                             "V:Verify Mode"]

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
            text: "W:All Word Stat"
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
            text: "Q:Close Window"
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
    }
}
