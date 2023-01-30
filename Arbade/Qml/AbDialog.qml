import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3

Dialog
{
    title: ""
    height: 160
    width: 300
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    focus: true
    x: (root.width - width) / 2
    y: (root.height - height) / 2
    property string dialog_text: ""
    property string dialog_label: ""
    property var auto_complete_list:[]


    Text
    {
        id: get_value_label
        anchors.left: parent.Left
        anchors.top: parent.top
        anchors.topMargin: 5
        font.pixelSize: 20
        text: dialog_label
        height: 40
    }
    TextField
    {
        id: get_value_input
        anchors.left: get_value_label.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        width: parent.width * 0.75

        text: "Input text"
        font.pixelSize: 16

        focus: true
        onFocusChanged: console.log("Focus changed " + focus)
        Keys.onReturnPressed: get_value_dialog.accept()

        onTextChanged:
        {
            suggestion_text.text = ""
            if( title==="Enter Category" && text.length )
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
            text += suggestion_text.text
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
        anchors.top: typed_text.top
        font.pixelSize: 16
        color: "#999"
    }

    onAccepted:
    {
        console.log("Accepted")
        dialog_text = get_value_input.text
    }

    onRejected:
    {
        console.log("Rejected")
    }

    onOpened:
    {
        get_value_input.text = ""
    }
}
