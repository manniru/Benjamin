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
                          word_count: w_count,
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

    function removeLast()
    {
        var len = wordbox_cl.children.length;
        wordbox_cl.children[len-1].destroy();
    }

    function getLineCount()
    {
        return wordbox_cl.children.length;
    }
}
