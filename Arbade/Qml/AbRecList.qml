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

    Component.onCompleted:
    {
        for( var i=0 ; i<10 ; i++ )
        {
            lm_reclist.append({"ni": 10-i});
        }
    }

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
}
