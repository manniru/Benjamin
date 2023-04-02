import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    height: wordbox_cl.height
    color: "transparent"

    property int start_num: 0
    property int last_box: 0
    property int commit: 0
    property int wl_count: 0
    property int set_focus: 0
    property string total_words: ""

    Column
    {
        id: wordbox_cl

        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: childrenRect.height
    }

    function addWord(w_text, w_count)
    {
        var i = wordbox_cl.children.length;

        var word_i = i+start_num;
        var comp_name = "WordLine" + word_i;
        var comp = Qt.createComponent("AbWordLine.qml");
        comp.createObject(wordbox_cl, {width:wordbox_cl.width,
                          word_id: word_i,
                          word_text: w_text,
                          word_number: w_count,
                          objectName: comp_name});
        editor_box.wordAdded(word_i);
    }

    function isFull()
    {
        var count = wordbox_cl.children.length;
        if( count>19 )
        {
            return true;
        }
        return false;
    }

    function focusLine(id)
    {
        wordbox_cl.children[id-start_num].applyFocus();
    }

}
