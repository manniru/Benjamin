import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.12

Window
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

            if( event.key===Qt.Key_Escape )
            {
                accept("N");
            }
            else
            {
                accept("Y");
            }
            close();
            clearAll();
        }
    }

    Grid
    {
        id: wrong_grid
        rows: 6
        anchors.top: verify_label.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 25
        columnSpacing: 55
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
        var word_i = wrong_grid.children.length;
        var comp_name = "QueryLine" + word_i;
        var comp = Qt.createComponent("AbQueryLine.qml");
        comp.createObject(wrong_grid, {width: 300,
                          word_shortcut: shortcut,
                          word_text: w_word,
                          word_path: w_path,
                          objectName: comp_name});
//        wordAdded(word_i);
    }

    function addCompleted()
    {
        console.log(Screen.width, width)
    }

    function clearAll()
    {
        var len = wrong_grid.children.length;
        for( var i=0 ; i<len ; i++ )
        {
            wrong_grid.children[i].destroy();
        }
    }

    onWidthChanged: x = Screen.width / 2 - width / 2
    onHeightChanged: y = Screen.height / 2 - height / 2
}
