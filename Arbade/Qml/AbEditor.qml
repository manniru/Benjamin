import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Window
{
    id: container
    title: "Edit Word List"
    height: 600
    width: 1200
//    visible: true
    color: "#2e2e2e"

    property string dialog_text: ""
    property string dif_words: ""

    property string botton_border: "#bfbfbf"
    property string botton_text: "#b6b6b6"
    property color  botton_bg: "#4d4d4d"
    property color  botton_hbg: "#666"

    property string total_words: ""
    property int line_count: 0
    property int box_count: 0

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

            onWordBoxChanged:
            {
                var split_words = total_words.split("\n");
                split_words[id] = word;
                total_words = split_words.join("\n");
            }

            onNewBoxRequired:
            {
                lm_wordedit.append({sn: line_count, wl: "",
                                   ws: "0", lb: Boolean(true)});
            }

            onNewWord:
            {
                line_count++;
                total_words += "\n";
            }
        }
    }

    ListView
    {
        id: lv_wordedit

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        width: parent.width - 20 // 20 margins
        height: parent.height - 70 // 40 button 30 margins
        orientation: ListView.Horizontal
        spacing: 40
        clip: true
        contentWidth: childrenRect.width

        model: lm_wordedit
        delegate: ld_wordedit
    }

    Button
    {
        id: save_button
        text: "Save"
        height: 40
        width: parent.width/4 - 5
        anchors.left: parent.left
        anchors.leftMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        font.pixelSize: 20
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        palette.buttonText: botton_text
        background: Rectangle
        {
            anchors.fill: parent
            color: if( parent.hovered )
                   {
                       botton_hbg
                   }
                   else
                   {
                       botton_bg
                   }
            border.color: botton_border
        }

        onClicked:
        {
            var dif_result = getDiff();
            if( dif_result.length!==0 )
            {
                dif_words = dif_result.join("\n");
                wordlist_dialog.dialog_label = "Are you sure" +
                        " to change these words?\n\n" + dif_words;
                wordlist_dialog.visible = true;
            }
            else
            {
                editor_dialog.reject();
            }

        }
    }

    Button
    {
        id: close_button
        text: "Close"
        height: 40
        width: parent.width/4 - 5
        anchors.right: parent.right
        anchors.rightMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        font.pixelSize: 20
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        palette.buttonText: botton_text

        background: Rectangle
        {
            anchors.fill: parent;
            color: if( parent.hovered )
                   {
                       botton_hbg
                   }
                   else
                   {
                       botton_bg
                   }

            border.color: botton_border
        }

        onClicked:
        {
            reject();
        }
    }

    AbDiffAccept
    {
        id: wordlist_dialog

        onDialog_resultChanged:
        {
            if( dialog_result==="Y" )
            {
                editor_dialog.accept();
                root_scene.forceActiveFocus();
            }
            else if( dialog_result==="N" )
            {
                editor_dialog.reject();
                root_scene.forceActiveFocus();;
            }
            dialog_result = "";
        }
    }

    onVisibleChanged:
    {
        if( !visible )
        {
            wordlist_dialog.close();
        }
        else
        {
            loadWordBoxes();        }
    }

    function accept()
    {
        root_scene.wordlist = total_words;
        root_scene.difwords = dif_words;
        dif_words = "";
        close();
    }

    function reject()
    {
        close();
    }

    function getDiff()
    {
        var words_old = root_scene.wordlist.split("\n").filter(i => i);
        var words_new = total_words.split("\n").filter(i => i);
        var dif = [];
        var count = 0;

        for( var i=0 ; i<words_new.length ; i++ )
        {
            if( words_new[i]!==words_old[i] )
            {
                count += 1;
                dif.push(count.toString() + ". " + words_old[i] +
                         "(" + i.toString() + ")" +
                         " => " + words_new[i]);
            }
        }
        return dif;
    }

    function loadWordBoxes()
    {
        lm_wordedit.clear();
        var all_words = root_scene.wordlist.split("\n");
        total_words = root_scene.wordlist;
        line_count = all_words.length;
        var all_stat = root_scene.wordstat.split("\n");
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
            lm_wordedit.append({sn: start_index, wl: sliced_wl,
                                ws: sliced_ws, lb: 1?(i===box_count-1):0});
            start_index += box_size;
        }
    }
}
