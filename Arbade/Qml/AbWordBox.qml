import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    property string word_list: ""
    property string word_stat: ""
    property int start_num: 0
    property int last_box: 0
    property int commit: 0
    property int wl_count: 0

    signal wordBoxChanged(int id, string word)
    signal newBoxRequired()
    signal newWord()

    color: "transparent"

    ListModel
    {
        id: lm_wordbox
    }

    Component
    {
        id: ld_wordbox

        AbWordLine
        {
            width: parent.width
            word_id: wid
            word_text: wt
            word_count: wc

            onWordChanged:
            {
                var last_id = start_num+wl_count-1;
                var word_id_int = parseInt(word_id);
                console.log(word_id_int, last_id, last_box)
                if( word_id_int===last_id && last_box )
                {
                    if( wl_count===ab_const.ab_WORDEDIT_BOX_SIZE )
                    {
                        newBoxRequired();
                    }
                    else
                    {
                        wl_count += 1;
                        lm_wordbox.append({wid: zeroPad(1+parseInt(word_id)),
                                       wt: "", wc: ""});
                    }
                    newWord();
                }
                else if( word_id_int===last_id-1 && last_box )
                {
                    if( text_w.length===0 )
                    {
                        wl_count -= 1;
                        lm_wordbox.remove(wl_count);
                    }
                }

                wordBoxChanged(word_id, text_w);
                lm_wordbox.get(parseInt(word_id)-start_num).wt = text_w;
            }
        }
    }

    ListView
    {
        id: lv_wordbox

        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: childrenRect.height
        interactive: false

        model: lm_wordbox
        delegate: ld_wordbox
    }

    onCommitChanged: // commit changes after all property
    {
        updateWords();
    }

    function updateWords()
    {
        var line_count = lm_wordbox.count;
        var wl_split = word_list.split("\n");
        wl_count = wl_split.length;
        if( line_count ) // update phase
        {
            if( line_count>wl_count )
            {
                lm_wordbox.remove(wl_count, line_count-wl_count);
            }
            for( var j=0 ; j<wl_count ; j++ )
            {
                lm_wordbox.get(j).wt = wl_split[j];
            }
        }
        else // creation phase
        {
            var wlc_split = word_stat.split("\n");

            for( var i=0 ; i<wl_count ; i++ )
            {
                lm_wordbox.append({wid: zeroPad(i+start_num),
                                   wt: wl_split[i], wc: wlc_split[i]});
            }
        }
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
}
