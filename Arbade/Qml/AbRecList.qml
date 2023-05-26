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

    onFocus_indexChanged: activateFocus()

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

    function addLine(word, path_in)
    {
        var len = reclist_cl.children.length;
        var comp = Qt.createComponent("AbRecLine.qml");
        var comp_name = "RecLine" + len;
        var rec_line;
        if( len>0 )
        {
            var last_element = reclist_cl.children[len-1];
            rec_line = comp.createObject(reclist_cl, {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  path: path_in,
                                  objectName: comp_name});
            rec_line.anchors.bottom = last_element.top;

        }
        else
        {
            rec_line = comp.createObject(reclist_cl, {width: reclist_cl.width,
                                  height: 30,
                                  num_id: len,
                                  sample_text: word,
                                  path: path_in,
                                  objectName: comp_name});
            rec_line.anchors.bottom = reclist_cl.bottom;
        }
        reclist_cl.height = (len+1)*30;
        wordAdded(len);
    }

    function getChildLen()
    {
        return reclist_cl.children.length;
    }

    function activateFocus()
    {
        if( focus_index>=0 )
        {
            var elem = reclist_cl.children[focus_index];
            elem.forceActiveFocus();
        }
        else
        {
            focus_item.forceActiveFocus();
        }
    }

    function clearRecList()
    {
        var len = reclist_cl.children.length;
        for( var i=0 ; i<len ; i++ )
        {
            reclist_cl.children[i].destroy();
        }
        reclist_cl.height = 0;
    }

    function unfocus()
    {
        focus_index = -1;
    }

    function removeLine(id, f_focus)
    {
        var len = reclist_cl.children.length;
        var elem = reclist_cl.children[id];
        var next_elem;
        var prev_elem ;
        var i;

        if( id>0 && id<len-1 )
        {
            prev_elem = reclist_cl.children[id-1];
            next_elem = reclist_cl.children[id+1];
            next_elem.anchors.bottom = prev_elem.top;
            if( f_focus )
            {
                next_elem.forceActiveFocus();
            }
        }
        else if ( id===0 && len>1 )
        {
            next_elem = reclist_cl.children[1];
            next_elem.anchors.bottom = reclist_cl.bottom;
            if( f_focus )
            {
                next_elem.forceActiveFocus();
            }
        }
        if( id===(len-1) )
        {
            if( focus_index>=0 )
            {
                focus_index = len-2;
            }
            prev_elem = reclist_cl.children[id-1];
            if( f_focus && prev_elem!==undefined )
            {
                prev_elem.forceActiveFocus();
            }
        }

        reclist_cl.height -= 30;

        for( i=(id+1) ; i<len ; i++ )
        {
            elem = reclist_cl.children[i];
            elem.num_id = i-1;
        }

        reclist_cl.children[id].destroy();
    }
}
