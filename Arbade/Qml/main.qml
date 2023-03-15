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
    property string ab_address: ""
    property string ab_word_list: ""
    property string ab_word_stat: ""
    property string ab_auto_comp: ""
    property string ab_mean_var: ""
    property string ab_focus_text: ""
    property int ab_focus_word: -1
    property int ab_count: 0
    property int ab_total_count: 100
    property real ab_elapsed_time: 0
    property int ab_status: ab_const.ab_STATUS_STOP
    property real ab_rec_time: 3
    property int ab_num_words: 3
    property real ab_pause_time: 1.0
    property real ab_power: 0
    property int ab_verifier: 0
    property int ab_playkon: 0

    signal loadsrc()
    signal delFile()
    signal copyFile()
    signal loadWordList()
    signal sendKey(int key)
    signal setStatus(int st)
    signal setVerifier(int ver)
    signal setFocusWord(int fw)
    signal setTotalCount(int val)
    signal saveWordList(string wl)
    signal setCategory(string cat)
    signal setDifWords(string dif)

    onAb_verifierChanged:
    {
        setVerifier(ab_verifier);
    }

    onAb_focus_wordChanged:
    {
        setFocusWord(ab_focus_word);
    }

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

    onAb_word_statChanged:
    {
        editor_box.loadWordBoxes();
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
        property alias verifier:        root.ab_verifier
    }

    Item
    {
        id: focus_item
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
        height: childrenRect.height
        anchors.top: parent.top
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
        focus_word: ab_focus_text
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
        onDialog_valChanged:
        {
            if( title===category_title )
            {
                ab_category = dialog_val;
                setCategory(ab_category);
            }
            else if( title===cnt_title )
            {
                var total_count = parseInt(dialog_val);
                ab_total_count = total_count;
                setTotalCount(ab_total_count);
            }
            else if( title===focus_word_title )
            {
                if( dialog_val=="" )
                {
                    ab_focus_word = -1;
                }
                else
                {
                    ab_focus_word = parseInt(dialog_val);
                }
            }
        }
    }

    AbDialogWsl
    {
        id: dialog_wsl

        onDriveEntered:
        {

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

    MouseArea
    {
        anchors.fill: parent

        onClicked: focus_item.forceActiveFocus();
    }

    AbButton
    {
        id: save_button
        text: "Save"
        width: .7*ab_sidebar.width
        anchors.horizontalCenter: ab_sidebar.horizontalCenter
        anchors.top: ab_sidebar.bottom
        anchors.topMargin: 60
        enabled: false

        onClick:
        {
            editor_box.saveProcess();
        }
    }

    AbButton
    {
        id: reset_button
        text: "Reset"
        width: .7*ab_sidebar.width
        anchors.horizontalCenter: ab_sidebar.horizontalCenter
        anchors.top: save_button.bottom
        anchors.topMargin: 15
        enabled: false

        onClick:
        {
            editor_box.resetProcess();
        }
    }

    AbEditor
    {
        id: editor_box

        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: ab_sidebar.right
        anchors.bottom: ab_help.top
        anchors.bottomMargin: 65

        onUpdateWordList:
        {
            saveWordList(word_list);
        }
        onUpdateDifWords:
        {
            setDifWords(dif_words);
        }
        onEnableButtons:
        {
            save_button.enabled = enable;
            reset_button.enabled = enable;
        }
    }

    Audio
    {
        id: audioPlayer
        source: ab_address

        onStopped:
        {
            if( sig_del_file )
            {
                sig_del_file = 0;
                delFile();
            }
            else
            {
                copyFile();
            }

            loadsrc();
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
                    (ab_rec_time+ab_pause_time)
            if( play_pos>100 )
            {
                play_pos=100
            }
        }
    }

    //Fonts:
    FontLoader
    {
        id: fontRobotoRegular
        source: "qrc:/Roboto-Regular.ttf"
    }

    function execKey(key)
    {
        if( key===Qt.Key_O || key===Qt.Key_J || key===Qt.Key_K ||
            key===Qt.Key_Up || key===Qt.Key_Down ||
            key===Qt.Key_Left || key===Qt.Key_Right ||
            key===Qt.Key_W || key===Qt.Key_T )
        {
            sendKey(key);
        }
        else if( key===Qt.Key_Space )
        {
            if( ab_verifier )
            {
                if( ab_status===ab_const.ab_STATUS_STOP )
                {
                    loadsrc();
                }
                else
                {
                    audioPlayer.pause();
                    ab_status = ab_const.ab_STATUS_PAUSE;
                    setStatus(ab_status);
                    verify_dialog.dialog_label = "Are you sure "+
                                         "you want to delete?\n"+
                                         "( Yes:space / No:q )"
                    verify_dialog.open();
                    verify_dialog.forceActiveFocus();
                }
            }
            else
            {
                if( ab_status===ab_const.ab_STATUS_REC ||
                    ab_status===ab_const.ab_STATUS_BREAK )
                {
                    ab_status = ab_const.ab_STATUS_REQPAUSE;
                    setStatus(ab_status);
                }
                else if( ab_status===ab_const.ab_STATUS_PAUSE ||
                         ab_status===ab_const.ab_STATUS_STOP )
                {
                    ab_status = ab_const.ab_STATUS_REC;
                    setStatus(ab_status);
                }
            }
        }
        else if( key===Qt.Key_Q )
        {
            close();
        }
        else if( key===Qt.Key_S )
        {
            if( ab_verifier===0 )
            {
                get_value_dialog.title = get_value_dialog.category_title;
                get_value_dialog.dialog_label = get_value_dialog.value_label;
                get_value_dialog.dialog_text = get_value_dialog.category_title;
                get_value_dialog.visible = true;
            }
        }
        else if( key===Qt.Key_C )
        {
            get_value_dialog.title = get_value_dialog.cnt_title;
            get_value_dialog.dialog_label = get_value_dialog.value_label;
            get_value_dialog.dialog_text = get_value_dialog.cnt_title;
            get_value_dialog.visible = true;
        }
        else if( key===Qt.Key_F )
        {
            get_value_dialog.title = get_value_dialog.focus_word_title;
            get_value_dialog.dialog_label = get_value_dialog.id_label;
            get_value_dialog.dialog_text = get_value_dialog.focus_word_title;
            get_value_dialog.visible = true;
        }
        else if( key===Qt.Key_V )
        {
            ab_status = ab_const.ab_STATUS_STOP;
            setStatus(ab_status);
            audioPlayer.stop()
            if( ab_verifier )
            {
                ab_verifier = 0;
            }
            else
            {
                ab_verifier = 1;
            }
        }
    }

    function initWsl()
    {
        dialog_wsl.visible = true;
    }
}
