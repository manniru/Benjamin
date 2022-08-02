import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.4
import Qt.labs.settings 1.0

Window
{
    id: root

    visible: true
    height: 30
    x: 0
    y: 0
    title: "PolyBar"
    color: "#000000"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    // Fonts
    FontLoader
    {
        id: fontAwesomeSolid
        source: "qrc:/Fonts/fa6-solid.ttf"
    }
    FontLoader
    {
        id: fontAwesomeBrand
        source: "qrc:/Fonts/fa6-brands.ttf"
    }
//    FontLoader
//    {
//        id: fontAwesomeRegular
//        source: "qrc:/Fonts/fa6-reg.ttf"
//    }
//    FontLoader
//    {
//        id: fontRobotoMedium
//        source: "qrc:/Fonts/Roboto-Medium.ttf"
//    }
    FontLoader
    {
        id: fontRobotoRegular
        source: "qrc:/Fonts/Roboto-Regular.ttf"
    }

    // Main
    MmDateLabel
    {
        id: date_label
        height: parent.height
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 6
    }

    MmFlickable
    {
        id: left_bar
        height: parent.height
        anchors.left: parent.left
        anchors.right: date_label.left
        anchors.top: parent.top
        direction: Qt.LeftToRight
        objectName: "LeftBar"
    }

    MmFlickable
    {
        id: right_bar
        height: parent.height
        anchors.right: parent.right
        anchors.left: date_label.right
        anchors.top: parent.top
        direction: Qt.RightToLeft
        objectName: "RightBar"
    }
}
