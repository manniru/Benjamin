import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Window
{
    title: "Edit Word List"
    height: 600
    width: 700
    property string dialog_text: ""
    property string botton_border: "#bfbfbf"
    property string botton_text: "#b6b6b6"
    property string botton_bg: "#4d4d4d"
    property string area_text: "#c9c9c9"
    property int line_count: 0
    visible: true

    color: "#2e2e2e"

    ScrollView
    {
        id: scroll_view
        width: parent.width - 20 // 2*10 = margins
        height: parent.height - 70 // 3*10 = margins | 40 buttons
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.leftMargin: 10

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        clip: true

        Rectangle
        {
            id: linenum_bg
            width: 50
            height: linenum_lbl.height
            anchors.top: parent.top
            anchors.left: parent.left

            color: "#666666"
        }

        Label
        {
            id: linenum_lbl
            anchors.top: parent.top
            anchors.horizontalCenter: linenum_bg.horizontalCenter
            anchors.topMargin: 7

            font.pixelSize: 16
            text: "010"
            color: "#b4b4b4"
        }

        Rectangle
        {
            id: text_bg
            height: text_area.height
            anchors.top: parent.top
            anchors.left: linenum_bg.right
            width: scroll_view.width - 20
            color: "#4e4e4e"
        }

        TextArea
        {
            id: text_area
            Accessible.name: "document"
            width: scroll_view.width - 70
            anchors.left: linenum_lbl.right
            anchors.leftMargin: 30
            anchors.top: parent.top

            text: ""
            font.pixelSize: 16
            Component.onCompleted: forceActiveFocus()
            focus: true
            color: area_text
            onTextChanged:
            {
                line_count = (text.match(/\n/g) || []).length + 1
            }

        }
    }

    Button
    {
        id: save_button
        text: "Save"
        height: 40
        width: scroll_view.width/2 - 5
        anchors.left: scroll_view.left
        anchors.top: scroll_view.bottom
        anchors.topMargin: 10
        font.pixelSize: 20
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        palette.buttonText: botton_text
        background: Rectangle
        {
            anchors.fill: parent
            color: botton_bg
            border.color: botton_border
        }

        onClicked:
        {
            accept();
        }
    }

    Button
    {
        id: close_button
        text: "Close"
        height: 40
        width: scroll_view.width/2 - 5
        anchors.right: scroll_view.right
        anchors.top: scroll_view.bottom
        anchors.topMargin: 10
        font.pixelSize: 20
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        palette.buttonText: botton_text
        background: Rectangle
        {
            anchors.fill: parent
            color: botton_bg
            border.color: botton_border
        }

        onClicked:
        {
            reject();
        }
    }

    onVisibleChanged:
    {
        updateText()
    }

    onLine_countChanged:
    {
        var line_text = ""

        for( var i=0 ; i<line_count ; i++ )
        {
            line_text += zeroPad(i) + "\n"
        }
        linenum_lbl.text = line_text
    }

    function accept()
    {
        console.log("Saved")
        root_scene.wordlist = text_area.getText(
                                0,text_area.length)
        close()
    }

    function reject()
    {
        console.log("Rejected")
        close()
    }

    function zeroPad(num)
    {
        var zero = 3 - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }

    function updateText()
    {
        text_area.text = root_scene.wordlist
//        console.log(text_area.text)
    }
}
