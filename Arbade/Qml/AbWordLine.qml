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
    property int    word_id: 0
    property string word_text: ""
    property string word_phoneme: ""
    property int    word_count: 0
    property bool   line_hovered: false
    property bool   wrong_phoneme: false
    property bool   control_pressed: false
    property bool   read_only_line: false

    signal wordChanged(int id, string text_w)
    signal saveWords()

    Rectangle
    {
        id: linenum_bg
        width: 50
        height: text_area.height
        anchors.top: parent.top
        anchors.left: parent.left

        color: if( wrong_phoneme )
               {
                   "#844e59"
               }
               else if( line_hovered )
               {
                   "#4e7084"
               }
               else
               {
                   "#666666"
               }

        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true

            onEntered:
            {
                if( !read_only_line )
                {
                    line_hovered = true;
                }
            }
            onExited:
            {
                line_hovered = false;
            }
        }
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
        color: if( wrong_phoneme )
               {
                   "#65474d"
               }
               else if( line_hovered )
               {
                   "#475a65"
               }
               else
               {
                   "#4e4e4e"
               }
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
        visible: !line_hovered
        readOnly: read_only_line

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
            if( event.key===Qt.Key_Control )
            {
                control_pressed = true;
            }
            else if( event.key===Qt.Key_S )
            {
                saveWords();
            }
            else if( event.key===Qt.Key_Backspace && text==="" )
            {
                var id = word_id;
                var word_count = editor_box.wordCount();
                if( id===word_count-1 )
                {
                    editor_box.arrowPress(word_id, Qt.Key_Up);
                }
            }
            else if( event.key===Qt.Key_Up ||
                     event.key===Qt.Key_Down )
            {
                editor_box.arrowPress(word_id, event.key);
            }
        }

        Keys.onReleased:
        {
            if( event.key===Qt.Key_Control )
            {
                control_pressed = false;
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
        id: phoneme_lbl
        anchors.left: linenum_lbl.right
        anchors.leftMargin: 36
        anchors.top: parent.top
        anchors.topMargin: 0

        text: word_phoneme.toLowerCase()
        font.pixelSize: 16
        color: "#c9c9c9"
        visible: line_hovered
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
        text: "(" + word_count + ")"
        color:
        {
            var mean = parseInt(buttons_box.mean);
            var variance = parseInt(buttons_box.variance);
            if( word_count<mean-variance && !read_only_line )
            {
                "#cb6565"; // red
            }
            else if( word_count>mean+variance && !read_only_line)
            {
                "#80bf73"; // green
            }
            else
            {
                "#9a9a9a"; // gray
            }
        }
        visible: word_count!==-1 &&
                 line_hovered===false
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
