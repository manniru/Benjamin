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

    signal hoverChange(int h)
    signal click()

    color:
    {
        if( line_fcsd )
        {
            if( hovered )
            {
                "#3f6981"
            }
            else
            {
                "#315265"
            }
        }
        else if( hovered )
        {
            "#828282"
        }
        else if( line_hvrd )
        {
            "#696969"
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
            click();
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
        text: "\uf1f8"
        color:
        {
            if( hovered )
            {
                "#d9a74c"
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

