import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    width: 800
    height: save_button.height
    color: "transparent"

    property string mean
    property string variance
    property int    btn_width: 200
    property bool   btn_enable: false
    property color  color_text:         "#b2b2b2"
    property int    font_size:          24
    property string font_name_label:    fontRobotoRegular.name

    signal saveClicked()
    signal resetClicked()

    Text
    {
        id: mean_label

        text: "Mean: " + mean
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -350
    }

    AbButton
    {
        id: save_button
        text: "Save"
        width: btn_width
        enabled: btn_enable
        anchors.top: parent.top
        anchors.topMargin: -5
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -125

        onClick:
        {
            saveClicked();
        }
    }

    AbButton
    {
        id: reset_button
        text: "Reset"
        width: btn_width
        enabled: btn_enable
        anchors.top: parent.top
        anchors.topMargin: -5
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 125

        onClick:
        {
//            resetClicked();
        }
    }

    Text
    {
        id: var_label

        text: "Var: " + variance
        color: color_text
        font.pixelSize: font_size
        font.family: font_name_label

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 350
    }
}
