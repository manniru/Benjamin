import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.12

Window
{
    title: "Lexicon Error Rate"
    width: 1000
    height: 790
    color: "#2e2e2f"

    property int mean: 0
    property int ed_height: height
    property int count: 0
    property int reverse_order: 0

    signal updateLer()

    Rectangle
    {
        id: editor_title

        color: "#797979"

        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 20

        height: 25

        Text
        {
            text: "Lexicon Error Rate"
            color: "#e5e5e5"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 15
            font.pixelSize: 15
        }

        Text
        {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 15

            color: "#e5e5e5"
            text: "<Click R to Reverse>"

            font.pixelSize: 15
        }
    }

    Flickable
    {
        id: scroller
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: editor_title.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentWidth: ler_grid.childrenRect.width + 20
        clip : true
        ScrollBar.horizontal: ScrollBar
        {
            height: 10
            anchors.bottom: parent.bottom
        }

        property int scroll_speed: 30

        Grid
        {
            id: ler_grid
            rows: Math.floor(scroller.height/26)
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            columnSpacing: 40
            flow: GridLayout.TopToBottom
        }
    }

    MouseArea
    {
        anchors.fill: scroller

        onWheel:
        {
            if( wheel.angleDelta.y>0 )
            {
                if( scroller.contentItem.x+10<scroller.x )
                {
                    scroller.contentItem.x += scroller.scroll_speed;
                }
            }
            else
            {
                if( scroller.contentItem.x+scroller.contentWidth+10
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

    Item
    {
        focus: true
        Keys.onPressed:
        {
            if( event.key===Qt.Key_Escape )
            {
                close();
            }
            else if( event.key===Qt.Key_R )
            {
                reverse_order = !reverse_order;
                updateLer();
            }
        }
    }

    function addWord(w_text, w_count, w_wrong)
    {
        var len = ler_grid.children.length;
        var comp_name = "LerLine" + len;
        var comp = Qt.createComponent("AbLerLine.qml");
        comp.createObject(ler_grid, {width: 200,
                          ler: w_count,
                          word_text: w_text,
                          wrong_word: w_wrong,
                          objectName: comp_name});
    }

    function clearEditor()
    {
        var len = ler_grid.children.length;
        for( var i=0 ; i<len ; i++ )
        {
            ler_grid.children[i].destroy();
        }
    }

    function wordCount()
    {
        var word_count = ler_grid.children.length;
        return word_count;
    }

    onWidthChanged: x = Screen.width / 2 - width / 2
    onHeightChanged: y = Screen.height / 2 - height / 2
}
