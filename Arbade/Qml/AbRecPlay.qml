import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Rectangle
{
    property bool hovered
    property bool line_hvrd
    property bool line_fcsd
    property bool playing: false

    signal hoverChange(int h)
    signal clicked()

    color:
    {
        if( line_fcsd )
        {
            if( hovered )
            {
                "#4e6e80"
            }
            else
            {
                "#3e5765"
            }
        }
        else if( hovered )
        {
            "#757575"
        }
        else if( line_hvrd )
        {
            "#5c5c5c"
        }
        else
        {
            "transparent"
        }
    }

    MouseArea
    {
        anchors.fill: parent
        hoverEnabled: true
        onClicked:
        {
            playing = !playing;
        }
        onEntered:
        {
            hovered = true;
            hoverChange(true);
        }
        onExited:
        {
            hovered = false;
            hoverChange(false);
        }
    }

    Label
    {
        anchors.centerIn: parent

        font.pixelSize: 16
        font.family: fontAwesomeSolid.name
        text:
        {
            if( playing )
            {
                "\uf04c";
            }
            else
            {
                "\uf04b";
            }
        }
        color:
        {
            if( hovered && playing )
            {
                "#e1dfe2"
            }
            else if( hovered && !playing )
            {
                "#a5cca2"
            }
            else if( line_fcsd || line_hvrd )
            {
                "#9a9a9a"
            }
            else
            {
                "transparent"
            }
        }
    }
}

