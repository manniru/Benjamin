import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    height: contentItem.height
    width: contentItem.width
    visible: true
    color: "red"

    MouseArea
    {
        anchors.fill: parent
        onClicked: focus_item.forceActiveFocus();
    }

    ListModel
    {
        id: lm_reclist
    }

    Component
    {
        id: ld_reclist

        AbWordBox
        {
            id: wordbox_id

            width: 200
            word_list: wl
            word_stat: ws
            start_num: sn
            last_box: lb
            commit: com
            set_focus: sf

            onWordBoxChanged:
            {
                var split_words = total_words.split("\n");
                split_words[id] = word;
                total_words = split_words.join("\n");
                var verbose = 0;
                var dif_result = getDiff(verbose);
                if( dif_result.length )
                {
                    enableButtons(1);
                }
                else
                {
                    enableButtons(0);
                }
            }

            onNewBoxRequired:
            {
                lm_wordedit.get(box_count-1).lb = 0;
                lm_wordedit.append({"sn": word_count, "wl": "",
                                   "ws": "0", "lb": Boolean(true),
                                   "com": 1, "sf": 0});
            }

            onNewWord:
            {
                word_count++;
                total_words += "\n";
            }

            onSetFNum:
            {
                focused_line = id;
            }

            onRemoveBox:
            {

            }

            onLineRemoved:
            {
                word_count--;
                total_words = total_words.substring(0,total_words.length-1); // removes \n
                arrowPress(word_count-1, Qt.Key_Up);
            }

            onArrowPrsd:
            {
                arrowPress(id, direction);
            }
        }
    }

    Flickable
    {
        id: scroller
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width
        height: lv_wordedit.height
        contentWidth: lv_wordedit.childrenRect.width + 20
        clip : true

        property int scroll_speed: 30

        ListView
        {
            id: lv_reclist
            height: childrenRect.height
            anchors.fill: parent
            orientation: ListView.Horizontal
            spacing: 40
            clip: true
            contentWidth: childrenRect.width

            model: lm_wordedit
            delegate: ld_wordedit
        }
    }
}
