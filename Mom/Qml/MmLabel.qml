import QtQuick 2.0
import QtQuick.Controls 1.4
 import QtQuick.Controls.Styles 1.4

Rectangle {
    id: container

    property string color_background:   ""
    property string color_label:        ""
    property string color_underline:    ""
    property bool   underline:          false
    property string label_text:         ""
    property string label_action:       ""

    // Qml Signals
    signal labelClicked()

    width: label.contentWidth
    color: container.color_background

    Text
    {
        id: label
        text: container.label_text
        textFormat: Text.RichText
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -1
        font.pixelSize: 15
        color: container.color_label
    }

    Rectangle
    {
        id: rect_underline
        width: parent.width
        height: 3
        anchors.top: label.bottom
        anchors.left: parent.left
        color: container.color_underline
        visible: container.underline
    }

    MouseArea
    {
        anchors.fill: parent
        enabled: container.label_action != ""
        cursorShape: enabled ? Qt.PointingHandCursor:Qt.ArrowCursor

        onClicked: labelClicked()
    }
}
