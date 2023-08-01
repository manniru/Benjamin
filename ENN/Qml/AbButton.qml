import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

Button
{
    height: 40
    font.pixelSize: 18
    palette.buttonText:
    {
        if( enabled )
        {
            "#b6b6b6"
        }
        else
        {
            "#9c9c9c"
        }
    }

    signal click();

    background: Rectangle
    {
        anchors.fill: parent
        color:
        {
            if( parent.enabled )
            {
                if( parent.hovered )
                {
                    "#666"
                }
                else
                {
                    "#4d4d4d"
                }
            }
            else
            {
                "#404040"
            }
        }
        border.color:
        {
            if( parent.enabled )
            {
                "#bfbfbf"
            }
            else
            {
                "#969696"
            }
        }
    }

    onClicked:
    {
        if( enabled )
        {
            click();
        }
    }
}
