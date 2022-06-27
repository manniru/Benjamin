import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Rectangle
{
    property string cell_name: "lolo"
    property color  cell_color: "#7f000000"
    property int    cell_size: 18

    color: "transparent"

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle
    {
        color: cell_color
        width: 40
        height: 40
        radius: 15

        opacity: 1

        anchors.centerIn: parent
        Label
        {
            text: cell_name
            font.pixelSize: cell_size
            color: if( cell_color=="#7f000000")
                   {
                       "#aaa"
                   }
                   else
                   {
                       "#fff"
                   }

            anchors.centerIn: parent
        }
    }

}

