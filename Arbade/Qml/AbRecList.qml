import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
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

//    Component
//    {
//        id: ld_reclist

//        AbRecLine
//        {
//            id: wordbox_id



//            focused: fc

//            onLineClicked:
//            {
//                var len = word_samples.length;
//                lm_reclist.get(len-index).fc = true;
//                for( var i=0 ; i<len ; i++ )
//                {
//                    if( i!==index-1 )
//                    {
//                        lm_reclist.get(len-i-1).fc = false;
//                    }
//                }
//            }

//            onArrowClicked:
//            {
//                var len = word_samples.length;
//                if( key===Qt.Key_Down && index>1)
//                {
//                    lm_reclist.get(len-index+1).fc = true;
//                    lm_reclist.get(len-index).fc = false;
//                }
//                else if( key===Qt.Key_Up && index<len )
//                {
//                    lm_reclist.get(len-index-1).fc = true;
//                    lm_reclist.get(len-index).fc = false; // do not factor this line!!
//                }
//            }

//            onRemoveClicked:
//            {
//                var len = word_samples.length;
//                delSample(word_samples[len-index]);
//                word_samples.splice(len - index,1); // remove element
//                updateRecList();
//            }
//        }
//    }

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
