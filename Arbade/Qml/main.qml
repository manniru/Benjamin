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
    height: 790
    minimumHeight: 780
    minimumWidth: 760

    color: "#2e2e2e"
    title: "ArBade"
    visible: true

    property int flag_kesafat_kari: 0

    property int sig_del_file: 0
    property int default_func_v: ab_const.ab_VMODE_COPY
    property real played_time

    property string ab_words: ""
    property string ab_address: ""
    property string ab_auto_comp: ""
    property string ab_focus_text: ""
    property string ab_dif_words: ""
    property int ab_focus_word: -1
    property int ab_count: 0
    property int ab_total_count: 100
    property int ab_total_count_v: 100
    property int ab_all_stat: 0
    property real ab_elapsed_time: 0
    property int ab_status: ab_const.ab_STATUS_STOP
    property real ab_rec_time: 3
    property int ab_num_words: 3
    property int ab_num_words_v: 3
    property real ab_rec_pause: 1.0
    property real ab_verify_pause: 0.5
    property real ab_power: 0
    property int ab_verifier: 0
    property real ab_start_now: 0

    signal startPauseV()
    signal delVerifyFile()
    signal copyUnverifyFile()
    signal trashVerifyFile()
    signal deleteSample(string sample)
    signal sendKey(int key)
    signal setStatus(int st)
    signal verifierChanged()
    signal setFocusWord(int fw)
    signal saveWordList()
    signal setCategory()
    signal setDifWords()

    Component.onCompleted:
    {
        ab_start_now = Date.now();
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    onAb_statusChanged:
    {
        if( ab_verifier )
        {
            if( ( ab_status===ab_const.ab_STATUS_PLAY ) &&
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

    Settings
    {
        property alias totalcount:      root.ab_total_count
        property alias category_name:   editor_box.category
        property alias rectime:         root.ab_rec_time
        property alias numwords:        root.ab_num_words
        property alias pausetime:       root.ab_rec_pause
        property alias verifypause:     root.ab_verify_pause
        property alias focusword:       root.ab_focus_word
        property alias verifier:        root.ab_verifier
        property alias allstat:         root.ab_all_stat
        property alias function_v:      root.default_func_v
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
        id: status_bar
        objectName: "Status"
        height: 50
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        pause_time: if( ab_verifier )
                    {
                        ab_verify_pause
                    }
                    else
                    {
                        ab_rec_pause
                    }
        num_words: ab_num_words
        rec_time: ab_rec_time
        count_total: if( ab_verifier )
                     {
                         ab_total_count_v
                     }
                     else
                     {
                         ab_total_count
                     }
        focus_word: ab_focus_text
    }

    AbHelp
    {
        id: ab_help

        height: 80
        anchors.top: parent.top
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
        onAcceptDialog:
        {
            if( title===category_title )
            {
                editor_box.category = value;
                setCategory();
            }
            else if( title===cnt_title )
            {
                var total_count = parseInt(value);
                ab_total_count = total_count;
            }
            else if( title===focus_word_title )
            {
                if( value==="" )
                {
                    ab_focus_word = -1;
                }
                else
                {
                    ab_focus_word = parseInt(value);
                }
                setFocusWord(ab_focus_word);
            }
        }
    }

    AbDialogWsl
    {
        id: dialog_wsl
        objectName: "WslDialog"
    }

    AbQuery
    {
        id: verify_dialog
        objectName: "Query"

        onAccept:
        {
            if( result==="Y" )
            {
                startPauseV();
            }
            else if( result==="N" )
            {
                audioPlayer.play();
                ab_status = ab_const.ab_STATUS_PLAY;
                setStatus(ab_status);
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            focus_item.forceActiveFocus();
            rec_list.unfocus();

            if( console_box.visible )
            {
                console_box.visible = false;
            }
        }
    }

    AbEditor
    {
        id: editor_box
        objectName: "WordList"

        anchors.top: ab_help.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: rec_list.left
        anchors.rightMargin: 10
        anchors.bottom: buttons_box.top
        anchors.bottomMargin: 30

        onUpdateDifWords:
        {
//            ab_dif_words = dif_words;
//            setDifWords();
        }
        onEnableButtons:
        {
            buttons_box.btn_enable = enable;
        }
    }

    AbButtons
    {
        id: buttons_box
        objectName: "Buttons"
        anchors.bottom: status_bar.top
        anchors.bottomMargin: 10
        anchors.horizontalCenter: editor_box.horizontalCenter
    }

    AbRecList
    {
        id: rec_list
        objectName: "RecList"

        height: editor_box.height
        anchors.top: ab_help.bottom
        anchors.topMargin: 20
        anchors.bottom: buttons_box.top
        anchors.bottomMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 20
    }

    AbMessage
    {
        id: ab_message
        objectName: "Message"

        anchors.centerIn: parent
    }

    AbConsole
    {
        id: console_box
        objectName: "Console"

        anchors.top: editor_box.top
        anchors.left: editor_box.left
        anchors.right: editor_box.right
        anchors.bottom: buttons_box.bottom

        visible: false
    }

    AbRecPanel
    {
        objectName: "RecPanel"
        anchors.left: editor_box.left
        anchors.right: editor_box.right
        anchors.top: editor_box.top
        anchors.bottom: buttons_box.bottom

        visible: if( ab_status===ab_const.ab_STATUS_STOP )
                 {
                     false
                 }
                 else
                 {
                     true
                 }

        status: ab_status
        words: ab_words
        elapsed_time:
        {
            if( ab_verifier )
            {
                played_time
            }
            else
            {
                ab_elapsed_time
            }
        }
        power: ab_power
    }

    Audio
    {
        id: audioPlayer
        source: ab_address

        onStopped:
        {
            console.log("FKK", flag_kesafat_kari)
            if( flag_kesafat_kari==0 )
            {
                ab_status = ab_const.ab_STATUS_DECPAUESE;
                decide_timer.start();
            }
            else
            {
                flag_kesafat_kari = 0;
            }
        }
    }

    Timer
    {
        id: audio_timer
        interval: 50
        repeat: true

        onTriggered:
        {
            played_time = audioPlayer.position/audioPlayer.duration
            played_time *= 100;
            if( played_time>100 )
            {
                played_time = 100
            }
        }
    }

    Timer
    {
        id: decide_timer
        interval: ab_const.ab_DECIDE_PAUSE
        repeat: false

        onTriggered:
        {
            interval = ab_const.ab_DECIDE_PAUSE;
            decide_timer.stop();
            if( default_func_v===ab_const.ab_VMODE_COPY )
            {
                copyUnverifyFile();
            }
            else if( default_func_v===ab_const.ab_VMODE_WRONG )
            {
                delVerifyFile();
            }
            else
            {
                trashVerifyFile();
            }

            startPauseV();
        }
    }

    //Fonts:
    FontLoader
    {
        id: fontRobotoRegular
        source: "qrc:/Roboto-Regular.ttf"
    }

    FontLoader
    {
        id: fontAwesomeSolid
        source: "qrc:/fa6-solid.ttf"
    }

    function execKey(key)
    {
        if( key===Qt.Key_O || key===Qt.Key_W || key===Qt.Key_T )
        {
            if( key===Qt.Key_W )
            {
                ab_all_stat = !ab_all_stat;
            }

            sendKey(key);
        }
        else if( key===Qt.Key_Escape )
        {
            if( console_box.visible )
            {
                console_box.visible = false;
            }
            if( ab_status!==ab_const.ab_STATUS_STOP )
            {
                audioPlayer.pause();
                audioPlayer.seek(0);
                decide_timer.stop();
                ab_status = ab_const.ab_STATUS_STOP;
                setStatus(ab_status);
            }

            sendKey(key);
        }
        else if( key===Qt.Key_Left )
        {
            ab_rec_time -= .1;
        }
        else if( key===Qt.Key_Right )
        {
            ab_rec_time += .1;
        }
        else if( key===Qt.Key_K )
        {
            ab_num_words--;
        }
        else if( key===Qt.Key_J )
        {
            ab_num_words++;
        }
        else if( key===Qt.Key_Up )
        {
            if( ab_verifier )
            {
                ab_verify_pause += .1;
            }
            else
            {
                ab_rec_pause += .1;
            }
        }
        else if( key===Qt.Key_Down )
        {
            if( ab_verifier )
            {
                ab_verify_pause -= .1;
            }
            else
            {
                ab_rec_pause -= .1;
            }
        }
        else if( key===Qt.Key_Space )
        {
            if( ab_verifier )
            {
                if( ab_status===ab_const.ab_STATUS_STOP )
                {
                    ab_count = 0;
                    startPauseV();
                }
                else if( ab_status===ab_const.ab_STATUS_PAUSE )
                {
                    audioPlayer.play();
                    ab_status = ab_const.ab_STATUS_PLAY;
                    setStatus(ab_status);
                    decide_timer.stop(); /// IS IT NECCESSARY?
                }
                else
                {
                    decide_timer.stop();
                    if( audioPlayer.playbackState===Audio.PlayingState )
                    {
                        audioPlayer.pause();
                        audioPlayer.seek(0);
                    }

                    ab_status = ab_const.ab_STATUS_PAUSE;
                    setStatus(ab_status);
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
            ab_count = 0
            verifierChanged();
        }
        else if( key===Qt.Key_Z )
        {
            if( ab_verifier )
            {
                cleanStop();
                ab_status = ab_const.ab_STATUS_PAUSE;
                setStatus(ab_status);
                decide_timer.stop();
                verify_dialog.visible = true;
            }
        }
        else if( key===Qt.Key_R )
        {
            if( default_func_v===ab_const.ab_VMODE_COPY )
            {
                default_func_v = ab_const.ab_VMODE_TRASH;
            }
            else if( default_func_v===ab_const.ab_VMODE_TRASH )
            {
                default_func_v = ab_const.ab_VMODE_WRONG;
            }
            else // ab_VMODE_WRONG
            {
                default_func_v = ab_const.ab_VMODE_COPY;
            }
        }
    }

    function playkon()
    {
        audioPlayer.play();
    }

    function incCount()
    {
        ab_count++;
    }

    function cleanStop()
    {
        flag_kesafat_kari = 1;
        audioPlayer.stop();
    }

    function initWsl()
    {
        dialog_wsl.visible = true;
    }
}
