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

    signal wordAdded(int index)

    onFocus_indexChanged:
    {
        if( focus_index>=0 && focus_index<getChildLen() )
        {
            root.enn_category = reclist_cl.children[focus_index].sample_text;
        }
    }

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

        ScrollBar.vertical: ScrollBar
        {
            width: 10
            anchors.right: parent.right // adjust the anchor as suggested by derM
        }

        Rectangle
        {
            id: reclist_cl
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    function addLine(word)
    {
        var len = reclist_cl.children.length;
        var comp = Qt.createComponent("EnnWordLine.qml");
        var comp_name = "WordLine" + len;
        var word_line;
        if( len>0 )
        {
            var last_element = reclist_cl.children[len-1];
            word_line = comp.createObject(reclist_cl,
                                 {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  objectName: comp_name});
            word_line.anchors.top = last_element.bottom;

        }
        else
        {
            word_line = comp.createObject(reclist_cl,
                                 {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  objectName: comp_name});
            word_line.anchors.top = reclist_cl.top;
        }
        reclist_cl.height = (len+1)*30;
        wordAdded(len);
        focus_index = 0;
    }

    function getChildLen()
    {
        return reclist_cl.children.length;
    }

    function execKey(key)
    {
        if( key===Qt.Key_Right && focus_index<getChildLen()-1 )
        {
            focus_index++;
        }
        else if( key===Qt.Key_Left && focus_index>0 )
        {
            focus_index--;
        }
        root.enn_category = reclist_cl.children[focus_index].sample_text;
    }
}
