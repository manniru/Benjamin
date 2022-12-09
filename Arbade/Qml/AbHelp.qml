import QtQuick 2.0

Rectangle
{
    property string font_name_label:    fontRobotoRegular.name

    property int    text_heigh:         key_label.height

    property int    font_size:          28
    property color  color_text:         "#333"

    property string key_text:          ""
    property string description_text:          ""

    color: "transparent"

    Text
    {
        id: key_label

        text: key_text
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label
        lineHeight: 1.2
    }

    Text
    {
        text: description_text
        anchors.left: key_label.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 30
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label
        lineHeight: 1.2
    }
}
