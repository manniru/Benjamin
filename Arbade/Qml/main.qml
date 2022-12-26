import QtQuick 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.0
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import QtMultimedia 5.5
import OpenGLUnderQML 1.0
import QtQml 2.12

ApplicationWindow
{
    id: root
    property int window_base_width : 1650
    property int window_base_height: 750
    property int sig_del_file: 0
    property real play_pos: 0

    width: window_base_width
    height: window_base_height
    minimumHeight: 500
    minimumWidth: 760

    color: "#2e2e2e"
    title: "ArBade"
    visible: true

    AbScene
    {
        id: root_scene
        anchors.fill: parent

        count: 0
        totalcount: 100
        category: "sag"
        words: "<One> <Roger> <Spotify>"
        elapsedtime: 0
        status: ab_const.ab_STATUS_STOP
        rectime: 3
        numwords: 3
        pausetime: 1
        focus: true
        stat: "One: 10 Two: 13 ...\nAlpha: 22  ..."
        address: ""
        key: 0
        power: -45
        verifier: 0
        loadsrc: 0
        delfile: 0
        playkon: 0

        Keys.onPressed:
        {
            execKey(event.key);
        }

        onStatChanged:
        {
            ab_stat.setText(stat);
        }

        onPlaykonChanged:
        {
            audioPlayer.play();
        }

        onStatusChanged:
        {
            if( verifier )
            {
                if( (status===ab_const.ab_STATUS_PLAY ||
                     status===ab_const.ab_STATUS_BREAK) &&
                    !audio_timer.running )
                {
                    audio_timer.start();
                }
                else if( status===ab_const.ab_STATUS_PAUSE ||
                         status===ab_const.ab_STATUS_STOP )
                {
                    audio_timer.stop();
                }
            }
        }
    }

    AbStatus
    {
        id: ab_status
        width: 400
        anchors.top: parent.top
        anchors.bottom: ab_help.top
        anchors.left: parent.left
        anchors.leftMargin: 30

        pause_time: root_scene.pausetime
        num_words: root_scene.numwords
        rec_time: root_scene.rectime
        status: root_scene.status
        words: root_scene.words
        category:
        {
            if( root_scene.verifier===1 )
            {
                "unverified"
            }
            else
            {
                root_scene.category
            }
        }
        count: root_scene.count
        count_total: root_scene.totalcount
        elapsed_time:
        {
            if( root_scene.verifier===0 )
            {
                root_scene.elapsedtime
            }
            else
            {
                play_pos
            }
        }
        power: root_scene.power
//        focus_word:
    }

    AbStat
    {
        id: ab_stat

        anchors.top: parent.top
        anchors.bottom: ab_help.top
        anchors.right: parent.right
        anchors.left: ab_status.right
    }

    AbHelp
    {
        id: ab_help

        height: 120
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    AbConst
    {
        id: ab_const
    }

    AbDialog
    {
        id: get_value_dialog

        onDialog_textChanged:
        {
            if( title==="Enter Category" )
            {
                root_scene.category = dialog_text
            }
            else if( title==="Enter Count" )
            {
                root_scene.totalcount = parseInt(dialog_text)
            }
        }
    }

    AbQuery
    {
        id: verify_dialog

        onDialog_resultChanged:
        {
            if( dialog_result==="Y" )
            {
                sig_del_file = 1;
                audioPlayer.stop();
            }
            else if( dialog_result==="N" )
            {
                sig_del_file = 0;
                audioPlayer.stop();
            }
            root_scene.forceActiveFocus();
            dialog_result = ""
        }


    }

    Audio
    {
        id: audioPlayer
        source: root_scene.address

        onStopped:
        {
            if( sig_del_file )
            {
                sig_del_file = 0;
                root_scene.delfile = 1;
            }
            root_scene.loadsrc = 1;
            play_pos = 0;
        }
    }

    Timer
    {
        id: audio_timer
        interval: 50
        repeat: true

        onTriggered:
        {// (*100.0 -> 0-100 %) (/1000 -> ms->sec)
            play_pos += 50.0*100.0/1000.0/
                    (root_scene.rectime+root_scene.pausetime)
            if( play_pos>100 )
            {
                play_pos=100
            }
        }
    }

    Component.onCompleted:
    {
        root_scene.qmlcreated = 1
    }

    //Fonts:
    FontLoader
    {
        id: fontRobotoRegular
        source: "qrc:/Roboto-Regular.ttf"
    }

    function execKey(key)
    {
        if( key===Qt.Key_Up )
        {
            root_scene.pausetime += .1;
        }
        else if( key===Qt.Key_Down )
        {
            root_scene.pausetime -= .1;
        }
        else if( key===Qt.Key_Left )
        {
            root_scene.numwords--;
        }
        else if( key===Qt.Key_Right )
        {
            root_scene.numwords++;
        }
        else if( key===Qt.Key_K )
        {
            root_scene.rectime += .1;
        }
        else if( key===Qt.Key_J )
        {
            root_scene.rectime -= .1;
        }
        else if( key===Qt.Key_Space )
        {
            if( root_scene.verifier )
            {
                if( root_scene.status===ab_const.ab_STATUS_STOP )
                {
                    root_scene.loadsrc = 1;
                }
                else
                {
                    audioPlayer.pause();
                    root_scene.status = ab_const.ab_STATUS_PAUSE;
                    verify_dialog.open();
                    verify_dialog.forceActiveFocus();
                }
            }
            else
            {
                if( root_scene.status===ab_const.ab_STATUS_REC ||
                    root_scene.status===ab_const.ab_STATUS_BREAK )
                {
                    root_scene.status = ab_const.ab_STATUS_REQPAUSE;
                }
                else if( root_scene.status===ab_const.ab_STATUS_PAUSE ||
                         root_scene.status===ab_const.ab_STATUS_STOP )
                {
                    root_scene.status = ab_const.ab_STATUS_REC;
                }
            }
        }
        else if( key===Qt.Key_Q )
        {
            close();
        }
        else if( key===Qt.Key_Return )
        {
            if( root_scene.verifier===0 )
            {
                get_value_dialog.title = "Enter Category";
                get_value_dialog.dialog_label = "value"
                get_value_dialog.open();
            }
        }
        else if( key===Qt.Key_T )
        {
            root_scene.key = key;
        }
        else if( key===Qt.Key_C )
        {
            get_value_dialog.title = "Enter Count";
            get_value_dialog.dialog_label = "value"
            get_value_dialog.open();
        }
        else if( key===Qt.Key_V )
        {
            root_scene.status = ab_const.ab_STATUS_STOP;
            audioPlayer.stop()
            if( root_scene.verifier===1 )
            {
                root_scene.verifier = 0;
            }
            else
            {
                root_scene.verifier = 1;
            }
        }
    }
}
