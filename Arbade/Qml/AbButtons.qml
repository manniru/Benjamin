import QtQuick 2.0
import QtQuick.Layouts 1.2

Rectangle
{
    width: save_button.width + reset_button.width + 20
    height: save_button.height
    color: "transparent"

    property int btn_width: 200
    property bool btn_enable: false

    AbButton
    {
        id: save_button
        text: "Save"
        width: btn_width
        enabled: btn_enable
        anchors.top: parent.top
        anchors.left: parent.left

        onClick:
        {
            editor_box.saveProcess();
        }
    }

    AbButton
    {
        id: reset_button
        text: "Reset"
        width: btn_width
        enabled: btn_enable
        anchors.left: save_button.right
        anchors.leftMargin: 20
        anchors.top: parent.top

        onClick:
        {
            editor_box.resetProcess();
        }
    }
}
