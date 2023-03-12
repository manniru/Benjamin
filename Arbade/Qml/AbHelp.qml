import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          24
    property color  color_text:         "#9a9a9a"

    property var help_text: ["Space:Pause Recording","S:Set Category",
                             "Up:Increase Pause","Down:Decrease Pause",
                             "Right:Increase Word","Left:Decrease Word",
                             "C:Change Count Number","F:Focus Word",
                             "J:Decrease Rec Time", "K:Increase Rec Time",
                             "O:Open Category Directory", "T: Train",
                             "V:Verify Mode", "W:All Word Stat","Q:Close Window"]

    color: "#262626"
//    color: "yellow"

    GridLayout
    {
        columns: 5
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        columnSpacing: 20

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
    }
}
