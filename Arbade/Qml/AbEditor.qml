import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    height: 600
    width: 1200
    visible: true
    color: "#2e2e2e"

    property string dialog_text: ""
    property string dif_words: ""

    property string total_words: ""
    property int word_count: 0
    property int box_count: 0

    signal updateWordList(string word_list)
    signal updateDifWords(string dif_words)
    signal enableButtons(int enable)

    ListModel
    {
        id: lm_wordedit
    }

    Component
    {
        id: ld_wordedit

        AbWordBox
        {
            id: wordbox_id

            width: 200
            word_list: wl
            word_stat: ws
            start_num: sn
            last_box: lb
            commit: com

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
                                   "com": 1});
            }

            onNewWord:
            {
                word_count++;
                total_words += "\n";
            }
        }
    }

    Flickable
    {
        id: scroller
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height
        contentWidth: lv_wordedit.childrenRect.width + 20
        clip : true

        property int scroll_speed: 30

        ListView
        {
            id: lv_wordedit
            anchors.fill: parent
            orientation: ListView.Horizontal
            spacing: 40
            clip: true
            contentWidth: childrenRect.width

            model: lm_wordedit
            delegate: ld_wordedit
        }
    }

    MouseArea
    {
        anchors.fill: scroller

        onWheel:
        {
            if( wheel.angleDelta.y>0 )
            {
                if( scroller.contentItem.x+10<scroller.x )
                {
                    scroller.contentItem.x += scroller.scroll_speed;
                }
            }
            else
            {
                if( scroller.contentItem.x+scroller.contentWidth+10
                   >scroller.x+scroller.width)
                {
                    scroller.contentItem.x -= scroller.scroll_speed;
                }
            }
        }

        onClicked: mouse.accepted = false;
        onPressed: mouse.accepted = false;
        onReleased: mouse.accepted = false;
        onDoubleClicked: mouse.accepted = false;
        onPositionChanged: mouse.accepted = false;
        onPressAndHold: mouse.accepted = false;
    }

    AbDiffAccept
    {
        id: wordlist_dialog

        onDialog_resultChanged:
        {
            if( dialog_result==="Y" )
            {
                editor_box.accept();
            }
            dialog_result = "";
        }
    }

    function accept()
    {
        updateWordList(total_words);
        updateDifWords(dif_words);
        dif_words = "";
    }

    function getDiff(verbose)
    {
        var words_old = root.ab_word_list.split("\n").filter(i => i);
        var words_new = total_words.split("\n").filter(i => i);
        var dif = [];
        var count = 0;

        for( var i=0 ; i<words_new.length ; i++ )
        {
            if( words_new[i]!==words_old[i] )
            {
                count += 1;
                if( verbose )
                {
                    dif.push(count.toString() + ". " + words_old[i] +
                             "(" + i.toString() + ")" +
                             " => " + words_new[i]);
                }
                else
                {
                    dif.push(i.toString());
                }
            }
        }
        return dif;
    }

    function saveProcess()
    {
        var verbose = 1;
        var dif_result = getDiff(verbose);
        if( dif_result.length!==0 )
        {
            dif_words = dif_result.join("\n");
            wordlist_dialog.dialog_label = "Are you sure" +
                    " to change these words?\n\n" + dif_words;
            wordlist_dialog.visible = true;
        }
    }

    function resetProcess()
    {
        var all_words = root.ab_word_list.split("\n");
        total_words = root.ab_word_list;
        word_count = all_words.length;
        var all_count = all_words.length;
        var box_size = ab_const.ab_WORDEDIT_BOX_SIZE;
        var start_index = 0;
        box_count = Math.ceil(all_count/box_size);
        var box_count_old = lm_wordedit.count;
        if( box_count<box_count_old )
        {
            lm_wordedit.remove(box_count, box_count_old-box_count)
        }

        for( var i=0 ; i<box_count ; i++ )
        {
            var sliced_wl = all_words.slice(start_index,
                               start_index+box_size).join("\n");
            var is_last_box = (i===box_count-1);
            lm_wordedit.get(i).wl = sliced_wl;
            lm_wordedit.get(i).lb = is_last_box;
            lm_wordedit.get(i).com = lm_wordedit.get(i).com+1;
            start_index += box_size;
        }
    }

    function loadWordBoxes()
    {
        lm_wordedit.clear();
        var all_words = root.ab_word_list.split("\n");
        total_words = root.ab_word_list;
        word_count = all_words.length;
        var all_stat = root.ab_word_stat.split("\n");
        var all_count = all_words.length
        var box_size = ab_const.ab_WORDEDIT_BOX_SIZE;
        var start_index = 0;
        box_count = Math.ceil(all_count/box_size);

        for( var i=0 ; i<box_count ; i++ )
        {
            var sliced_wl = all_words.slice(start_index,
                               start_index+box_size).join("\n")
            var sliced_ws = all_stat.slice(start_index,
                               start_index+box_size).join("\n")
            lm_wordedit.append({"sn": start_index, "wl": sliced_wl,
                                "ws": sliced_ws, "lb": (i===box_count-1),
                                "com": 1});

            start_index += box_size;
        }
    }
}
