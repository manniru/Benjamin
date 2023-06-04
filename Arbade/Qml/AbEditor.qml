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

    property string category: ""
    property int ed_height: height
    property int focused_line: -1
    property int count: 0
    property bool read_only: false
    property string title_str:
    {
        if( root.ab_verifier===1 )
        {
            "Word Editor - Category: \"unverified\""
        }
        else if( root.ab_verifier===2 )
        {
            "Word Editor - Category: \"shit\""
        }
        else
        {
            "Word Editor - Category: \"" + category + "\""
        }
    }

    signal updateWordList()
    signal updateDifWords()
    signal enableButtons(int enable)
    signal wordAdded(int id)

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
            text: title_str
            color: "#e5e5e5"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 15
            font.pixelSize: 15
        }

        Text
        {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 15

            color: "#e5e5e5"
            text:
                if( !read_only )
                {
                    "Count: " + count
                }
                else
                {
                    ""
                }

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
        contentWidth: wordedit_grid.childrenRect.width + 20
        clip : true
        ScrollBar.horizontal: ScrollBar
        {
            height: 10
            anchors.bottom: parent.bottom
        }

        property int scroll_speed: 30

        Grid
        {
            id: wordedit_grid
            rows: Math.floor(ed_height/26)
            anchors.top: parent.top
            anchors.left: parent.left
            columnSpacing: 40
            flow: GridLayout.TopToBottom
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
                acceptSave();
            }
            dialog_result = "";
        }
    }

    function acceptSave()
    {
        updateDifWords();
        updateWordList();
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
            if( id===wordCount()-1 )
            {
                return;
            }
            focused_line = id+1;
        }
        wordedit_grid.children[focused_line].applyFocus();
    }

    function addWord(w_text, w_count, w_phoneme)
    {
        var word_i = wordedit_grid.children.length;
        var comp_name = "WordLine" + word_i;
        var comp = Qt.createComponent("AbWordLine.qml");
        comp.createObject(wordedit_grid, {width: 200,
                          word_id: word_i,
                          word_text: w_text,
                          word_phoneme: w_phoneme,
                          word_count: w_count,
                          objectName: comp_name,
                          read_only_line: read_only});
        wordAdded(word_i);
    }

    function clearEditor()
    {
        var len = wordedit_grid.children.length;
        for( var i=0 ; i<len ; i++ )
        {
            wordedit_grid.children[i].destroy();
        }
    }

    function removeWord()
    {
        var len = wordedit_grid.children.length;
        wordedit_grid.children[len-1].destroy();
    }

    function wordCount()
    {
        var word_count = wordedit_grid.children.length;
        return word_count;
    }
}
