import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQml 2.3

Window
{
    id: window_main
    property string  m_text: "mamad joon"
    property int     count_x: 15
    property int     count_y: 15
    property real    o_state: 1
    property color   ch_cell_color: "#af000000"
    property color   ch_active_color: "#7f5f6f00"
    property string  ch_buffer: ""


    signal eKeyPressed(int key)

    title: "Chess"
    width: 800
    height: 600
    visible: true
    color: "transparent"
    opacity: 1.0

    Item
    {
        focus: true
        Keys.onPressed:
        {
            if( ch_buffer.length===0 )
            {
                ch_buffer += String.fromCharCode(event.key);
                state0Highlight();
            }
            else if( ch_buffer.length===1 )
            {
                if ( event.key===Qt.Key_Backspace )
                {
                    ch_buffer = ch_buffer.substring(0, ch_buffer.length-1);
                    resetHighlight();
                }
                else
                {
                    ch_buffer += String.fromCharCode(event.key);
                    state1Highlight(event.key);
                }
            }
            else if( ch_buffer.length===2 )
            {
                if ( event.key===Qt.Key_Backspace )
                {
                    ch_buffer = ch_buffer.substring(0, ch_buffer.length-1);
                    state0Highlight();
                }
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
        repeat: true
//        running: true
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

    function state1Highlight(ev_key)
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
