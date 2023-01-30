import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

Rectangle
{
    id: container
    property string font_name_label:    fontRobotoRegular.name

    property int    font_size:          28
    property color  color_text:         "#b3b3b3"

    property var grid_text: ["apples", "oranges", "pears"]

//    color: "green"
    color: "transparent"

    ScrollView
    {
        id: scroller
        width: parent.width
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        clip : true

        property int scroll_speed: 30

        GridLayout
        {
            id: stat_grid
            columns: 12
            anchors.fill: parent
            columnSpacing: 20

            Repeater
            {
                model: grid_text
                Text
                {
                    text: modelData
                    color: color_text
                    font.pixelSize: font_size
                    font.family: font_name_label

                    textFormat: Text.RichText
    //                verticalAlignment: Text.AlignVCenter
    //                Layout.alignment: Qt.AlignVCenter
                }
            }
        }
    }

    MouseArea
    {
        anchors.fill: scroller

        onWheel:
        {
            if( wheel.angleDelta.y>0 )
            {
                if( scroller.contentItem.x+30<scroller.x )
                {
                    scroller.contentItem.x += scroller.scroll_speed;
                }
            }
            else
            {
                if( scroller.contentItem.x+scroller.contentWidth+30
                   >scroller.x+scroller.width)
                {
                    scroller.contentItem.x -= scroller.scroll_speed;
                }
            }
        }

        onClicked: mouse.accepted = false;
        onPressed: mouse.accepted = false;
        onReleased: mouse.accepted = false;
        onDoubleClicked: mouse.accepted = false;
        onPositionChanged: mouse.accepted = false;
        onPressAndHold: mouse.accepted = false;
    }

    function setText(text)
    {
        grid_text = text.split('!').filter(Boolean); // filter(Boolean) to remove empty parts
    }
}
