import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Window
{
    id: window_main
    property string  m_text: "mamad joon"
    property int     count_x: 26
    property int     count_y: 26
    property real    o_state: 1
    property real    o_speed: 0.25
    property color   ch_cell_color: "#777"
    property color   ch_active_color: "red"
    property string  ch_buffer: ""
    property variant ch_cell_char: [ "a", "b", "c", "d", "e", "f", "g", "h", "i",
                                     "j", "k", "l", "m", "n", "o", "p", "q", "r",
                                     "s", "t", "u", "v", "w", "x", "y", "z" ]

    signal eKeyPressed(int key)

    title: "Chess"
    width: 800
    height: 600
    visible: true
    color: "black"
    opacity: 0.9

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
        columns: count_y

        anchors.fill: parent
    }


    Component.onCompleted:
    {
        for( var i=0 ; i<count_x ; i++ )
        {
            for( var j=0 ; j<count_y ; j++ )
            {
                var query = "
                import QtQuick 2.2;
                ChCell
                {
                    width:1000
                    height:1000
                    cell_name: \"" + ch_cell_char[i] + ch_cell_char[j] + "\"
                }";

                var object = Qt.createQmlObject(query, g_layout);
            }
        }
    }

    Timer
    {
        interval: 50
        repeat: true
        //        running: true

        onTriggered:
        {
            if( window_main.opacity>=0.99 )
            {
                o_state = -1
                if( window_main.opacity>1 )
                {
                    window_main.opacity = 1
                }
            }
            else if( window_main.opacity<=0.01 )
            {
                o_state = 1
                if( window_main.opacity<0 )
                {
                    window_main.opacity = 0
                }
            }

            var offset;
            if( window_main.opacity<0.5 )
            {
                offset = o_speed*window_main.opacity;
            }
            else
            {
                offset = o_speed-o_speed*window_main.opacity;
            }
            if( offset===0 )
            {
                offset = 0.001
            }
            offset *= o_state;
            window_main.opacity += offset;
        }
    }


    function highlightRow(row)
    {
        for( var j=0 ; j<count_y ; j++ )
        {
            g_layout.children[row*count_x+j].color = ch_active_color;
        }
    }

    function hightlightCell(row, col)
    {
        g_layout.children[row*count_x+col].color = ch_active_color;
    }

    function resetHighlight()
    {
        for( var i=0 ; i<count_x ; i++ )
        {
            for( var j=0 ; j<count_y ; j++ )
            {
                g_layout.children[i*count_x+j].color = ch_cell_color;
            }
        }
    }


    function state0Highlight()
    {
        for( var i=0 ; i<count_x ; i++ )
        {
            if ( g_layout.children[i*count_x].cell_name[0]===ch_buffer[0].toLowerCase() )
            {
                highlightRow(i);
            }
        }
    }

    function state1Highlight(ev_key)
    {
        resetHighlight();

        for( var i=0 ; i<count_x ; i++ )
        {
            for( var j=0 ; j<count_y ; j++ )
            {
                if ( (g_layout.children[i*count_x+j].cell_name[1]===ch_buffer[1].toLowerCase()) &&
                        (g_layout.children[i*count_x+j].cell_name[0]===ch_buffer[0].toLowerCase()) )
                {
                    hightlightCell(i, j);
                }
            }
        }
    }
}
