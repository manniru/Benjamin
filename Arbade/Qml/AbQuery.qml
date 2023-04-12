import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3

ApplicationWindow
{
    title:  if( root.default_func_v )
            {
                "Copy File ?"
            }
            else
            {
                "Delete File ?"
            }
    height: verify_label.height + wrong_comb.height + 50
    width: 350
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    color: "#2e2e2f"

    signal accept(string result)

    property string dialog_result: ""
    property string font_name_label:    fontRobotoRegular.name
    property color  color_text:         "#9a9a9a"

    property var wrong_text: ["<wrong> <wrong> <sag>",
                             "<wrong> <gas> <sag>",
                             "<marg> <wrong> <sag>",
                             "<wrong> <wrong> <wrong>",
                             "<wrong> <gas> <wrong>",
                             "<marg> <gas> <wrong>",
                             "<marg> <wrong> <wrong>"]

    Text
    {
        id: verify_label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        text:   if( root.default_func_v )
                {
                    "Are you sure "+
                    "you want to copy?\n"+
                    "( Yes:z / No:q )"
                }
                else
                {
                    "Are you sure "+
                    "you want to delete?\n"+
                    "( Yes:z / No:q )"
                }

        lineHeight: 1.4
        color: "#b4b4b4"

        Keys.onPressed:
        {
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

    Column
    {
        id: wrong_comb
        anchors.top: verify_label.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 20
        spacing: 10

        Repeater
        {
            model: wrong_text
            AbButton
            {
                width: wrong_comb.width
                text: modelData
                font.pixelSize: 18
                font.family: font_name_label
            }
        }
    }

    Component.onCompleted:
    {
        verify_label.forceActiveFocus();
    }

    function generateWrongComb(words)
    {
        console.log("generateWrongComb", root.ab_words, words)
    }
}
