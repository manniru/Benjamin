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
    property var word_samples: []

    signal delSample(string sample)

    ListModel
    {
        id: lm_reclist
    }

    Component
    {
        id: ld_reclist

        AbRecLine
        {
            id: wordbox_id
            width: lv_reclist.parent.width
            height: 30

            num_id: ni
            sample_text: st
            focused: fc

            onLineClicked:
            {
                var len = word_samples.length;
                lm_reclist.get(len-index).fc = true;
                for( var i=0 ; i<len ; i++ )
                {
                    if( i!==index-1 )
                    {
                        lm_reclist.get(len-i-1).fc = false;
                    }
                }
            }

            onArrowClicked:
            {
                var len = word_samples.length;
                if( key===Qt.Key_Down && index>1)
                {
                    lm_reclist.get(len-index+1).fc = true;
                    lm_reclist.get(len-index).fc = false;
                }
                else if( key===Qt.Key_Up && index<len )
                {
                    lm_reclist.get(len-index-1).fc = true;
                    lm_reclist.get(len-index).fc = false; // do not factor this line!!
                }
            }

            onRemoveClicked:
            {
                var len = word_samples.length;
                delSample(word_samples[len-index]);
                word_samples.splice(len - index,1); // remove element
                updateRecList();
            }
        }
    }

    ListView
    {
        id: lv_reclist
        anchors.fill: parent
        contentHeight: 30*count + 20
        clip: true
//            contentWidth: childrenRect.width

        model: lm_reclist
        delegate: ld_reclist
    }

    function updateRecList()
    {
        lm_reclist.clear();
        var len = word_samples.length;
        for( var i=0 ; i<len ; i++ )
        {
            lm_reclist.append({"ni": len-i, "st": word_samples[i],
                              "fc": false});
        }
    }
}
