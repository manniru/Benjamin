import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    id: wordline
    height: text_area.height
    property int word_id: 0
    property string word_text: ""
    property int word_number: 0

    signal wordChanged(int id, string text_w)
    signal arrowPressed(int direction)
    signal removeLine()

    Rectangle
    {
        id: linenum_bg
        width: 50
        height: text_area.height
        anchors.top: parent.top
        anchors.left: parent.left

        color: "#666666"
    }

    Label
    {
        id: linenum_lbl
        anchors.top: parent.top
        anchors.horizontalCenter: linenum_bg.horizontalCenter
        anchors.topMargin: 3

        font.pixelSize: 16
        text: zeroPad(word_id)
        color: "#b4b4b4"
    }

    Rectangle
    {
        id: text_bg
        height: text_area.height
        anchors.top: parent.top
        anchors.left: linenum_bg.right
        width: wordline.width - 20
        color: "#4e4e4e"
    }

    TextArea
    {
        id: text_area
        Accessible.name: "document"
        width: wordline.width - 70
        anchors.left: linenum_lbl.right
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: -2

        text: word_text
        font.pixelSize: 16
        padding: 3
        focus: false
        selectByMouse: true
        color: "#c9c9c9"
        selectedTextColor: "#333"
        selectionColor: "#ccc"

        onTextChanged:
        {
            word_text = text;
            wordChanged(word_id, text);
        }

        Keys.onEscapePressed:
        {
            focus_item.forceActiveFocus();
        }

        Keys.onPressed:
        {
            if( event.key===Qt.Key_Backspace && text==="" )
            {
                var id = word_id;
                if( id===editor_box.word_count-2 )
                {
                    removeLine();
                }
                else if( id===editor_box.word_count-1 )
                {
                    editor_box.arrowPress(word_id, event.key);
                }
            }
            else if( event.key===Qt.Key_Up ||
                     event.key===Qt.Key_Down )
            {
                editor_box.arrowPress(word_id, event.key);
            }
        }

        onFocusChanged:
        {
            if( focus )
            {
                editor_box.focused_line = word_id;
            }
        }
    }

    Label
    {
        id: stat_lbl
        anchors.top: parent.top
        anchors.right: text_bg.right
        anchors.rightMargin: 20
        anchors.topMargin: 3
        horizontalAlignment: Text.AlignHCenter

        font.pixelSize: 16
        text: "(" + word_number + ")"
        color:
        {
            var mean = parseInt(buttons_box.mean);
            var variance = parseInt(buttons_box.variance);
            if( word_number<mean-variance )
            {
                "#cb6565"; // red
            }
            else if( word_number>mean+variance )
            {
                "#80bf73"; // green
            }
            else
            {
                "#9a9a9a"; // gray
            }
        }
    }

    onWord_textChanged:
    {
        text_area.text = word_text;
    }

    function applyFocus()
    {
        text_area.forceActiveFocus();
        text_area.cursorPosition = text_area.text.length;
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
}
