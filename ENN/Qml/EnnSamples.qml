import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    visible: true
    color: "transparent"

    property int ed_height: height
    property int focused_line: -1
    property int index: 0
    property int count: 0
    property string title_str:
    {
        "Sample Viewer - Word: \"" + root.enn_category + "\""
    }

    signal sampleAdded(int id)

    Rectangle
    {
        id: editor_title

        color: "#797979"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10

        height: 25

        Text
        {
            text: title_str
            color: "#e5e5e5"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 15
            font.pixelSize: 15
        }

        Text
        {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 15

            color: "#e5e5e5"
            text: "Index: " + index + "/" + count
            font.pixelSize: 15
        }
    }

    Grid
    {
        id: sample_grid
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: editor_title.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        columns: 5
        rows: 2
        columnSpacing: 40
        rowSpacing: 50
        flow: GridLayout.LeftToRight
    }

    function addSample(sample_name, sample_path)
    {
        var word_i = sample_grid.children.length;
        var comp_name = "Sample" + word_i;
        var comp = Qt.createComponent("EnnSample.qml");
        comp.createObject(sample_grid, {word_id: word_i,
                          word_text: sample_name,
                          objectName: comp_name,
                          path: sample_path});
        sampleAdded(word_i);
    }
}
