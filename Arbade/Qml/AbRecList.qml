import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    width: 390
    visible: true
    color: "transparent"
    property int focus_index: -1
    signal delSample(string sample)

    Rectangle
    {
        id: reclist_title

        color: "#797979"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 0

        height: 25

        Text
        {
            text: "Record List History"
            color: "#e5e5e5"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 15
            font.pixelSize: 15
        }
    }

    Flickable
    {
        anchors.left: parent.left
        anchors.top: reclist_title.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        contentHeight: reclist_cl.height
        clip: true

        Rectangle
        {
            id: reclist_cl
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    function addLine(word, path_in)
    {
        var len = reclist_cl.children.length;
        var comp = Qt.createComponent("AbRecLine.qml");
        console.log(path_in);
        var rec_line;
        if( len>0 )
        {
            var last_element = reclist_cl.children[len-1];
            rec_line = comp.createObject(reclist_cl, {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  path: path_in});
            rec_line.anchors.bottom = last_element.top;

        }
        else
        {
            rec_line = comp.createObject(reclist_cl, {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  path: path_in});
            rec_line.anchors.bottom = reclist_cl.bottom;
        }
        reclist_cl.height = (len+1)*30;
    }
}
