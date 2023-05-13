import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQml 2.3

Window
{
    id: window_main
    property int     count_x : 36
    property int     count_y : 26
    property bool    ch_timer: true
    property real    o_state: 1
    property color   ch_cell_color:   "#cf000000" // "#cf000000"
    property color   ch_active_color: "#7f5f6f00"
    property color   ch_drag_color:   "#7f6f0064"
    property string  ch_buffer: ""

    signal eKeyPressed(int key)

    title: "Chess"
    width: 800
    height: 600
    visible: true
    color: "transparent"
    opacity: 0.8
    flags: Qt.WA_X11NetWmWindowTypeDock

    Item
    {
        focus: true
        Keys.onPressed:
        {
            if( event.key!==Qt.Key_Escape )
            {
                keyHandler(event.key);
            }
            eKeyPressed(event.key);
        }
    }

    GridLayout
    {
        id: g_layout
        columns: count_x

        anchors.fill: parent
    }

    Component.onCompleted:
    {
        for( var i=0 ; i<count_y ; i++ )
        {
            for( var j=0 ; j<count_x ; j++ )
            {
                var ch_name  = String.fromCharCode(i+65);
                if( j<10 )
                {
                    ch_name += String.fromCharCode(j+48);
                }
                else
                {
                    ch_name += String.fromCharCode(j+55);
                }

                var query = "
                import QtQuick 2.2;
                ChCell
                {
                    width:1000
                    height:1000
                    cell_name: \"" + ch_name + "\"
                }";

                var object = Qt.createQmlObject(query, g_layout);
            }
        }
    }

    OpTimer
    {
        interval: 50
        repeat:   true
        running:  ch_timer
    }

    function keyHandler(key_event)
    {
        var min_key = '0'.charCodeAt()-1;
        var max_key = 'Z'.charCodeAt()+1;
        if( key_event===Qt.Key_F1 )
        {
            ch_cell_color = "#cf002422"
        }
        else if ( key_event===Qt.Key_Backspace )
        {
            if( ch_buffer.length )
            {
                ch_buffer = ch_buffer.substring(0, ch_buffer.length-1);
            }
        }
        else if( key_event>min_key && key_event<max_key )
        {
            ch_buffer += String.fromCharCode(key_event);
        }
        updateState();
    }

    function updateState()
    {
        if( ch_buffer.length===0 )
        {
            resetHighlight();
        }
        else if( ch_buffer.length===1 )
        {
            state0Highlight();
        }
        else if( ch_buffer.length===2 )
        {
            state1Highlight();
        }
    }

    function highlightRow(row)
    {
        for( var j=0 ; j<count_x ; j++ )
        {
            g_layout.children[row*count_x+j].cell_color = ch_active_color;
        }
    }

    function hightlightCell(row, col)
    {
        g_layout.children[row*count_x+col].cell_color = ch_active_color;
    }

    function lightDragCell(row, col)
    {
        g_layout.children[row*count_x+col].cell_color = ch_drag_color;
    }

    function resetHighlight()
    {
        for( var i=0 ; i<count_y ; i++ )
        {
            for( var j=0 ; j<count_x ; j++ )
            {
                g_layout.children[i*count_x+j].cell_color = ch_cell_color;
            }
        }
    }

    function state0Highlight()
    {
        for( var i=0 ; i<count_y ; i++ )
        {
            if ( g_layout.children[i*count_x].cell_name[0]===ch_buffer[0] )
            {
                highlightRow(i);
            }
        }
    }

    function state1Highlight()
    {
        resetHighlight();

        for( var i=0 ; i<count_y ; i++ )
        {
            for( var j=0 ; j<count_x ; j++ )
            {
                if ( (g_layout.children[i*count_x+j].cell_name[1]===ch_buffer[1]) &&
                        (g_layout.children[i*count_x+j].cell_name[0]===ch_buffer[0]) )
                {
                    hightlightCell(i, j);
                }
            }
        }
    }
}
