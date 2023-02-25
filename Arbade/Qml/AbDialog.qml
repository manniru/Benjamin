import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Window
{
    title: ""
    height: 120
    width: 300
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    color: "#2e2e2e"
    property string botton_border: "#bfbfbf"
    property string botton_text: "#b6b6b6"
    property color  botton_bg: "#4d4d4d"
    property color  botton_hbg: "#666"
    property string dialog_text: ""
    property string dialog_label: ""
    property var auto_complete_list:[]

    property string category_title: "Set Category"
    property string focus_word_title: "Enter Focus Word ID"
    property string cnt_title: "Enter Count"
    property string value_label: "value"
    property string id_label: "ID"


    Text
    {
        id: get_value_label
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 17
        font.pixelSize: 20
        text: dialog_label
        height: 40
        color: "#b4b4b4"
    }

    TextField
    {
        id: get_value_input
        anchors.left: get_value_label.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 15
        width: parent.width * 0.7
        background: Rectangle
        {
            anchors.fill: parent
            color: "#666666"
        }

        text: "Input text"
        font.pixelSize: 16

        focus: true
        Keys.onReturnPressed: get_value_dialog.accept()

        onTextChanged:
        {
            suggestion_text.text = ""
            if( title===category_title && text.length )
            {
                for( var i=0 ; i<auto_complete_list.length ; i++ )
                {
                    var category = auto_complete_list[i];
                    if( text===category.substr(0,text.length) )
                    {
                        suggestion_text.text = category.substr(text.length)
                        break
                    }
                }
            }
        }

        Keys.onTabPressed:
        {
            text += suggestion_text.text;
            accept();
        }
    }

    Label
    {
        id: typed_text
        anchors.left: get_value_input.left
        anchors.leftMargin: 10
        anchors.verticalCenter: get_value_input.verticalCenter
        text: get_value_input.text
        font.pixelSize: 16
        color: "transparent"
    }

    Label
    {
        id: suggestion_text
        anchors.left: typed_text.right
        anchors.leftMargin: 1
        anchors.verticalCenter: get_value_input.verticalCenter
        font.pixelSize: 16
        color: "#999"
        text: "khar"
    }

    AbButton
    {
        id: save_button
        text: "Save"
        width: parent.width/4 - 5
        anchors.left: parent.left
        anchors.leftMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

        onClick:
        {
            accept();
        }
    }

    AbButton
    {
        id: close_button
        text: "Close"
        width: parent.width/4 - 5
        anchors.right: parent.right
        anchors.rightMargin: parent.width/4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

        onClick:
        {
            reject();
        }
    }

    function accept()
    {
        dialog_text = get_value_input.text;
        close();
    }

    function reject()
    {
        close();
    }

    onVisibleChanged:
    {
        if( visible )
        {
            get_value_input.text = "";
        }
    }
}
