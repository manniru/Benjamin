import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.12

Window
{
    title: "Lexicon Error Rate"
    width: 1000
    height: 790
    color: "#2e2e2f"

    AbEditor
    {
        id: test_stat
        objectName: "TestStat"

        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30

        title_str: "Lexicon Error Rate"
        read_only: true
    }

    onWidthChanged: x = Screen.width / 2 - width / 2
    onHeightChanged: y = Screen.height / 2 - height / 2
}
