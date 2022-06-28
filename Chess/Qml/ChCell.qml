import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Rectangle
{
    property int    cell_size : 18
    property color  cell_color: ch_cell_color
    property string cell_name : "lolo"

    color: "transparent"

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle
    {
        width: 40
        height: 40

        radius: 15
        color : cell_color
        opacity: 1

        anchors.centerIn: parent
        Label
        {
            text: cell_name
            font.pixelSize: cell_size
            color: if( cell_color=="#7f000000")
                   {
                       "#ccc"
                   }
                   else
                   {
                       "#fff"
                   }

            anchors.centerIn: parent
        }
    }

}

