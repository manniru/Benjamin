import QtQuick 2.0
import QtQuick.Controls 1.4
 import QtQuick.Controls.Styles 1.4

Rectangle {
    property string color_background:   ""
    property string color_label:        ""
    property string color_underline:    ""
    property bool   underline:          false
    property string label_text:         ""
    property string label_action_l:       ""
    property string label_action_r:       ""
    property string label_action_m:       ""
    property string label_action_u:       ""
    property string label_action_d:       ""
    property string label_pre:  "<div style = 'font-family: Roboto, \"Font Awesome 6 Brands Regular\", \"Font Awesome 6 Pro Solid\"'>"
    property string label_post: "</div>"

    // Qml Signals
    signal labelClicked(string action)

    width: label.contentWidth
    color: color_background

    Text
    {
        id: label
        text: label_pre + label_text + label_post
        textFormat: Text.RichText
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -1
        font.pixelSize: 15
        color: color_label
    }

    Rectangle
    {
        id: rect_underline
        width: parent.width
        height: 3
        anchors.top: label.bottom
        anchors.left: parent.left
        color: color_underline
        visible: underline
    }

    MouseArea
    {
        anchors.fill: parent
        enabled: label_action_l != ""
        cursorShape: enabled ? Qt.PointingHandCursor:Qt.ArrowCursor

        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        onClicked:
        {
            // 'mouse' is a MouseEvent argument passed into the onClicked signal handler
            if ( mouse.button===Qt.LeftButton )
            {
                labelClicked(label_action_l)
            }
            else if ( mouse.button===Qt.RightButton )
            {
                labelClicked(label_action_r)
            }
            else if ( mouse.button===Qt.MiddleButton )
            {
                labelClicked(label_action_m)
            }
        }
    }
}
