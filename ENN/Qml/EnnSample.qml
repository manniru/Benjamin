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
    width: 200
    height: 240
    visible: word_id>=0
    color: "transparent"

    property int    word_id: 0
    property string word_text: ""
    property string path: ""

    Rectangle
    {
        id: sample_image
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: parent.height - 40
        color: "#666666"

        Image
        {
            source: "image://samples/" +
                        root.enn_category + "/" + path
            anchors.fill: parent
        }
    }

    Label
    {
        id: linenum_lbl
        anchors.top: sample_image.bottom
        anchors.horizontalCenter: sample_image.horizontalCenter
        anchors.topMargin: 3
        width: parent.width

        font.pixelSize: 16
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Label.WrapAnywhere
        text: word_id + ". " + word_text
        color: "#b4b4b4"
    }
}
