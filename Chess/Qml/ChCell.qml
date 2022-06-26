import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Rectangle
{
    property string cell_name: "lolo"

    color: "#777"

    Layout.fillHeight: true
    Layout.fillWidth: true

    Label
    {
        text: cell_name
        font.pixelSize: 20
        color: "#fff"
        anchors.centerIn: parent
    }
}

