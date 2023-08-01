import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    width: time_label.width
    height: time_label.height
    color: "transparent"

    property string font_name_label:    fontRobotoRegular.name
    property int    font_size:          24
    property real   time:               0
    property color  color_text:         "#b2b2b2"

    Text
    {
        id: time_label

        text: "Time: [                               ]"
        anchors.left: parent.left
        anchors.top: parent.top
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label
        lineHeight: 1.2
    }

    Rectangle
    {
        anchors.left: parent.left
        anchors.leftMargin: 76
        anchors.top: parent.top
        anchors.topMargin: 2
        color: color_text
        width: (time/100)*178
        height: 25
    }
}
