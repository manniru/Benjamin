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
    color: "transparent"

    property string dif_words: ""
    property string category: ""
    property int word_count: 0
    property int box_count: 0
    property int focused_line: -1

    signal updateWordList()
    signal updateDifWords(string dif_words)
    signal enableButtons(int enable)
    signal wordAdded(int id)

    MouseArea
    {
        anchors.fill: parent
        onClicked: focus_item.forceActiveFocus();
    }

    Rectangle
    {
        id: editor_title

        color: "#797979"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10

        height: 25

        Text
        {
            text: if( root.ab_verifier===1 )
                  {
                      "Word Editor - Category: \"unverified\""
                  }
                  else
                  {
                      "Word Editor - Category: \"" + category + "\""
                  }

            color: "#e5e5e5"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 15
            font.pixelSize: 15
        }
    }

    Flickable
    {
        id: scroller
        anchors.left: parent.left
        anchors.top: editor_title.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width
        height: wordedit_row.height
        contentWidth: wordedit_row.childrenRect.width + 20
        clip : true

        property int scroll_speed: 30

        Row
        {
            id: wordedit_row
            anchors.top: parent.top
            anchors.left: parent.left
            spacing: 40
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
        updateWordList();
        updateDifWords(dif_words);
        dif_words = "";
    }

    function launchDialog(dif_words)
    {
        wordlist_dialog.dialog_label = "Are you sure" +
                " to change these words?\n" + dif_words;
        wordlist_dialog.visible = true;
    }

    function arrowPress(id, direction)
    {
        if( focused_line===-1 )
        {
            return;
        }
        if( direction===Qt.Key_Up )
        {
            if( id===0 )
            {
                return;
            }
            focused_line = id-1;
        }
        else if( direction===Qt.Key_Down )
        {
            if( id===word_count-1 )
            {
                return;
            }
            focused_line = id+1;
        }
        var box_id = Math.floor(focused_line/
                                     ab_const.ab_WORDEDIT_BOX_SIZE);
        var box = wordedit_row.children[box_id];
        box.focusLine(focused_line);
    }

    function addWord(w_text, w_count)
    {
        var box_id = wordedit_row.children.length-1;
        var comp;

        if( box_id<0 )
        {
            comp = Qt.createComponent("AbWordBox.qml");
            comp.createObject(wordedit_row, {width: 200});
            box_id = 0;
        }

        var box = wordedit_row.children[box_id];
//        console.log("we are", w_text, w_count, box_id, box);

        if( box.isFull() )
        {
            box_id++;
            var start_number = box_id * 21;
            comp = Qt.createComponent("AbWordBox.qml");
            comp.createObject(wordedit_row, {width: 200, start_num: start_number});
        }

        box.addWord(w_text, w_count);
    }

    function clearEditor()
    {
        var len = wordedit_row.children.length;
        for( var i=0 ; i<len ; i++ )
        {
            wordedit_row.children[i].destroy();
        }
//        console.log(wordedit_row.children.length)
    }
}
