import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          28
    property color  color_text:         "#b3b3b3"

    property var grid_text: ["apples", "oranges", "pears"]

//    color: "green"
    color: "transparent"

    GridLayout
    {
        columns: 12
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        columnSpacing: 20

        Repeater
        {
            model: grid_text
            Text
            {
                text: modelData
                color: color_text
                font.pixelSize: font_size
                font.family: font_name_label

                textFormat: Text.RichText
//                verticalAlignment: Text.AlignVCenter
//                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    function setText(text)
    {
        grid_text = text.split('!').filter(Boolean); // filter(Boolean) to remove empty parts
    }
}
