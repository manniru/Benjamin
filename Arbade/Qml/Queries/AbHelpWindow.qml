import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Window 2.12

Window
{
    title: "Keyboard Shortcuts"
    width: help_grid.width + 60
    height: help_grid.height + 110

    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          20
    property color  color_text:         "#9a9a9a"

    property var help_text: ["Space: Rec/Pause",
                             "Up/Down: Pause Time",
                             "Right/Left: Rec Time",
                             "A: Show LER",
                             "B: Find Bad", "C: Change Count",
                             "D: Gen E-Sample", "E: Train ENN",
                             "F: Set Focus-Word",
                             "H: Shit Mode", "J/K: Word Num",
                             "O: Open Category",
                             "P: Show Terminal", "Q: Close Window",
                             "S: Set Category", "T: Train",
                             "V: Verify Mode"]

    color: "#262626"
//    color: "yellow"

    GridLayout
    {
        id: help_grid
        rows: 5
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        columnSpacing: 20
        flow: GridLayout.TopToBottom
        focus: true

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
            text:
            {
                if( root.default_func_v===ab_const.ab_VMODE_WRONG )
                {
                    "R: Wrong Mode"
                }
                else if( root.default_func_v===ab_const.ab_VMODE_COPY )
                {
                    "R: Copy Mode"
                }
                else
                {
                    "R: Trash Mode"
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
            text: "W: All Stat"
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
            text: "Z: Delete Sample"
            color: color_text
            font.pixelSize: font_size
            font.family: font_name_label
        }

        Keys.onEscapePressed:
        {
            close();
        }
    }
}
