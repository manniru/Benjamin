import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Rectangle
{
    property int    cell_size  : 35
    property int    cell_fsize : 16
    property color  cell_color: ch_cell_color
    property string cell_name : "lolo"

    color: "transparent"

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle
    {
        width:  cell_size
        height: cell_size

        radius: 15
        color : cell_color
        opacity: 1

        anchors.centerIn: parent
        Label
        {
            text: cell_name
            font.pixelSize: cell_fsize
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

