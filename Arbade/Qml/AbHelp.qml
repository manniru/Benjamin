import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          24
    property color  color_text:         "#9a9a9a"

    property var help_text: ["Up:Increase Pause","Down:Decrease Pause",
                             "Right:Increase Word",
                             "Left:Decrease Word",
                             "K:Increase Rec Time","J:Decrease Rec Time",
                             "Space:Pause Recording",
                             "Enter:Change Category",
                             "T:Open Category Directory",
                             "C:Change Count Number",
                             "F:Focus Word", "W:Word List",
                             "V:Verify Mode","Q:Close Window"]

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
