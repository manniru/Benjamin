import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3

ApplicationWindow
{
    title: "Wrong Selection Dialog"
    width: wrong_grid.width + 80
    height: wrong_grid.height + 110
    color: "#2e2e2f"

    signal accept(string result)
    signal generate()
    signal keyPress(int key)

    property string dialog_result: ""
    property string font_name_label:    fontRobotoRegular.name
    property color  color_text:         "#9a9a9a"

    onVisibleChanged:
    {
        if( visible )
        {
            generate();
        }
    }

    Text
    {
        id: verify_label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        text: "Please select the best describing form:"

        lineHeight: 1.4
        color: "#b4b4b4"

        Keys.onPressed:
        {
            keyPress(event.key);

            if( event.key===Qt.Key_Z ||
                event.key===Qt.Key_Y )
            {
                accept("Y");
                close();
            }
            else
            {
                accept("N");
                close();
            }
        }
    }

    Grid
    {
        id: wrong_grid
        rows: 6
        anchors.top: verify_label.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        columnSpacing: 40
        flow: GridLayout.TopToBottom
    }

    Component.onCompleted:
    {
        verify_label.forceActiveFocus();
    }

    function generateWrongComb(words)
    {
        console.log("generateWrongComb", root.ab_words, words);
    }

    function addForm(w_word, w_path, shortcut)
    {
        console.log("word = ", w_word, "w_path =", w_path,
                    "shortcut = ", shortcut);

        var word_i = wrong_grid.children.length;
        var comp_name = "QueryLine" + word_i;
        var comp = Qt.createComponent("AbQueryLine.qml");
        comp.createObject(wrong_grid, {width: 200,
                          word_shortcut: shortcut,
                          word_text: w_word,
                          word_path: w_path,
                          objectName: comp_name});
//        wordAdded(word_i);
    }
}
