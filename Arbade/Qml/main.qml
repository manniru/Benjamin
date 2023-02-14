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
import QtQml 2.12

ApplicationWindow
{
    id: root

    width: 1650
    height: 750
    minimumHeight: 500
    minimumWidth: 760

    color: "#2e2e2e"
    title: "ArBade"
    visible: true

    property int sig_del_file: 0
    property real play_pos: 0

    property string ab_category: "online"
    property string ab_words: "<One> <Roger> <Spotify>"
    property string ab_stat: "One: 10 Two: 13 ...\nAlpha: 22  ..."
    property string ab_address: ""
    property string ab_focus_word: "<empty>"
    property string ab_word_list: ""
    property string ab_word_stat: ""
    property string ab_auto_comp: ""
    property string ab_mean_var: ""
    property int ab_count: 0
    property int ab_total_count: 100
    property real ab_elapsed_time: 0
    property int ab_status: ab_const.ab_STATUS_STOP
    property real ab_rec_time: 3
    property int ab_num_words: 3
    property real ab_pause_time: 1
    property real ab_power: 0
    property int ab_verifier: 0
    property int ab_playkon: 0

    signal loadsrc()
    signal delFile()
    signal copyFile()
    signal loadWordList()
    signal saveWordList()
    signal sendKey(int key)
    signal setStatus(int st)
    signal setVerifier(int ver)
    signal setTotalCount(int val)
    signal setCategory(string cat)
    signal setDifWords(string dif)
    signal setFocusWord(string fw)

    onAb_playkonChanged:
    {
        audioPlayer.play();
    }

    onAb_statusChanged:
    {
        if( ab_verifier )
        {
            if( ( ab_status===ab_const.ab_STATUS_PLAY ||
                  ab_status===ab_const.ab_STATUS_BREAK) &&
                !audio_timer.running )
            {
                audio_timer.start();
            }
            else if( ab_status===ab_const.ab_STATUS_PAUSE ||
                     ab_status===ab_const.ab_STATUS_STOP )
            {
                audio_timer.stop();
            }
        }
    }

    onAb_mean_varChanged:
    {
        ab_sidebar.mean = ab_mean_var.split("!")[0];
        ab_sidebar.variance = ab_mean_var.split("!")[1];
    }

    Settings
    {
        property alias totalcount:      root.ab_total_count
        property alias category_name:   root.ab_category
        property alias rectime:         root.ab_rec_time
        property alias numwords:        root.ab_num_words
        property alias pausetime:       root.ab_pause_time
        property alias focusword:       root.ab_focus_word
    }

    Item
    {
        focus: true
        Keys.onPressed:
        {
            execKey(event.key);
        }
    }

    AbStatus
    {
        id: ab_sidebar
        width: 400
        anchors.top: parent.top
        anchors.bottom: ab_help.top
        anchors.left: parent.left
        anchors.leftMargin: 50

        pause_time: ab_pause_time
        num_words: ab_num_words
        rec_time: ab_rec_time
        status: ab_status
        words: ab_words
        category:
        {
            if( ab_verifier===1 )
            {
                "unverified"
            }
            else
            {
                ab_category
            }
        }
        count: ab_count
        count_total: ab_total_count
        elapsed_time:
        {
            if( ab_verifier===0 )
            {
                ab_elapsed_time
            }
            else
            {
                play_pos
            }
        }
        power: ab_power
        focus_word: ab_focus_word
    }

    AbStat
    {
        anchors.top: parent.top
        anchors.bottom: ab_help.top
        anchors.right: parent.right
        anchors.left: ab_sidebar.right

        grid_text: ab_stat.split('!').filter(Boolean);
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

        auto_complete_list: ab_auto_comp.split("!").
                                filter(i => i);
        onDialog_textChanged:
        {
            if( title===category_title )
            {
                ab_category = dialog_text;
                setCategory(dialog_text);
            }
            else if( title===cnt_title )
            {
                var total_count = parseInt(dialog_text);
                ab_total_count = total_count;
                setTotalCount(total_count);
            }
            else if( title===focus_word_title )
            {
                if( dialog_text=="" )
                {
                    ab_focus_word = ab_focus_word;
                    setFocusWord("<empty>");
                }
                else
                {
                    ab_focus_word = dialog_text;
                    setFocusWord(dialog_text);
                }
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
            dialog_result = ""
        }
    }

    AbEditor
    {
        id: editor_dialog

        dialog_text: root_scene.wordlist
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
        root_scene.wordlist = "request data";
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
                    verify_dialog.dialog_label = "Are you sure "+
                                         "you want to delete?\n"+
                                         "( Yes:space / No:q )"
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
        else if( key===Qt.Key_S )
        {
            if( root_scene.verifier===0 )
            {
                get_value_dialog.title = get_value_dialog.category_title;
                get_value_dialog.dialog_label = get_value_dialog.value_label;
                get_value_dialog.visible = true;
            }
        }
        else if( key===Qt.Key_T )
        {
            root_scene.key = key;
        }
        else if( key===Qt.Key_C )
        {
            get_value_dialog.title = get_value_dialog.cnt_title;
            get_value_dialog.dialog_label = get_value_dialog.value_label;
            get_value_dialog.visible = true;
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
        else if( key===Qt.Key_F )
        {
            get_value_dialog.title = get_value_dialog.focusword_title;
            get_value_dialog.dialog_label = get_value_dialog.id_label;
            get_value_dialog.visible = true;
        }
        else if( key===Qt.Key_W )
        {
            root_scene.wordlist = "request data";
            editor_dialog.visible = true;
        }
    }
}
