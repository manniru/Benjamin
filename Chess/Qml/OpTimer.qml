import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQml 2.3

Timer
{
    property real op_min   : 0.1
    property real op_max   : 0.6
    property real o_speed  : 0.2
    property int  delay    : 10
    property int  delay_buf: 0

    property real op_min_e: op_min+0.01
    property real op_max_e: op_max-0.01

    onTriggered:
    {
        if( window_main.opacity>op_max_e )
        {
            if( delay_buf<delay*2 )
            {
                delay_buf++;
            }
            else
            {
                o_state = -1;
                delay_buf = 0;
            }
        }
        else if( window_main.opacity<op_min_e )
        {
            if( delay_buf<delay )
            {
                delay_buf++;
            }
            else
            {
                o_state = 1;
                delay_buf = 0;
            }
        }
        if( window_main.opacity>op_max )
        {
            window_main.opacity = op_max;
        }
        if( window_main.opacity<op_min )
        {
            window_main.opacity = op_min;
        }

        var offset;
        if( window_main.opacity<0.5 )
        {
            offset = o_speed*(window_main.opacity-op_min);
        }
        else
        {
            offset = o_speed*(op_max-window_main.opacity);
        }
        if( offset===0 )
        {
            offset = 0.001;
        }
        offset *= o_state;
        window_main.opacity += offset;
    }
}
