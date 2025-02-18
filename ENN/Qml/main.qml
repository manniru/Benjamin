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

    property int flag_clean_stop: 0

    property int sig_del_file: 0
    property int default_func_v: ab_const.ab_VMODE_COPY
    property real played_time
    property int ctrl_pressed: 0
    property int audio_paused: 0

    property string ab_words: ""
    property string ab_address: ""
    property string ab_auto_comp: ""
    property string ab_focus_text: ""
    property string ab_focus_text_v: ""
    property int ab_focus_word: -1
    property int ab_focus_word_v: -1
    property int ab_count: 0
    property int ab_total_count: 100
    property int ab_total_count_v: 100
    property int ab_all_stat: 0
    property real ab_elapsed_time: 0
    property int ab_status: ab_const.ab_STATUS_STOP
    property real ab_rec_time: 3
    property int ab_num_words: 3
    property int ab_verifier: 0
    property int ab_verify_id: 0
    property real ab_rec_pause: 1.0
    property real ab_verify_pause: 0.5
    property real ab_power: 0
    property string enn_category: ""

    signal startPauseV()
    signal delVerifyFile()
    signal copyUnverifyFile()
    signal trashVerifyFile()
    signal deleteSample(string sample)
    signal sendKey(int key)
    signal setStatus(int st)
    signal verifierChanged()
    signal focusWordChanged()
    signal setCategory()
    signal saveWordList()
    signal readLerDiff()
    signal generateESamples()
    signal trainWithWord(int word_id)
    signal updateRecList()
    signal positionUpdated(int direction)
    signal loadEnnSamples(string cat)

    Component.onCompleted:
    {
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

    onEnn_categoryChanged:
    {
        loadEnnSamples(enn_category);
    }

    Settings
    {
        property alias totalcount:      root.ab_total_count
        property alias rectime:         root.ab_rec_time
        property alias numwords:        root.ab_num_words
        property alias pausetime:       root.ab_rec_pause
        property alias verifypause:     root.ab_verify_pause
        property alias focusword:       root.ab_focus_word
        property alias focusword_v:     root.ab_focus_word_v
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
        Keys.onReleased:
        {
            if( event.key===Qt.Key_Control )
            {
                ctrl_pressed = 0;
            }
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
        focus_word: if( ab_verifier )
                    {
                        ab_focus_text_v
                    }
                    else
                    {
                        ab_focus_text
                    }
    }

    AbTopBar
    {
        id: top_bar
        objectName: "TopBar"

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
                root.enn_category = value.toLowerCase();
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
                    if( ab_verifier )
                    {
                        ab_focus_word_v = -1;
                    }
                    else
                    {
                        ab_focus_word = -1;
                    }
                }
                else
                {
                    if( ab_verifier )
                    {
                        ab_focus_word_v = parseInt(value);
                    }
                    else
                    {
                        ab_focus_word = parseInt(value);
                    }
                }
                focusWordChanged();
                updateRecList();
            }
        }
    }

    AbEnnWord
    {
        id: enn_word_dialog

        onAcceptDialog:
        {
            var w_id = parseInt(value);
            if( isNaN(w_id) )
            {
                ab_message.message = "Word id should be an integer.";
            }
            trainWithWord(w_id);
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

            if( console_box.visible )
            {
                console_box.visible = false;
            }
        }
    }

    EnnSamples
    {
        id: sample_box
        objectName: "SamplesList"

        anchors.top: top_bar.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: word_list.left
        anchors.rightMargin: 10
        anchors.bottom: buttons_box.top
        anchors.bottomMargin: 30
    }

    AbButtons
    {
        id: buttons_box
        objectName: "Buttons"
        anchors.bottom: status_bar.top
        anchors.bottomMargin: 10
        anchors.horizontalCenter: sample_box.horizontalCenter
    }

    EnnWordList
    {
        id: word_list
        objectName: "wordList"

        height: sample_box.height
        anchors.top: top_bar.bottom
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

        anchors.top: sample_box.top
        anchors.left: sample_box.left
        anchors.right: sample_box.right
        anchors.bottom: buttons_box.bottom

        visible: false
    }

    AbHelpWindow
    {
        id: help_win
    }

    AbLerStat
    {
        id: ler_stat
        objectName: "LerStat"
        onUpdateLer:
        {
            readLerDiff();
        }
    }

    AbESampleQuery
    {
        id: e_sample_query
        visible: false
        onAcceptDialog:
        {
            generateESamples();
        }
    }

    AbVerifyEFalse
    {
        id: verify_efalse_query
        objectName: "EFalseDialog"
        onAcceptDialog:
        {
            ab_status = ab_const.ab_STATUS_STOP;
            setStatus(ab_status);
            audioCleanStop();
            ab_verifier = 2;
            ab_count = 0;
            console_box.visible = false;
            verifierChanged();
        }
    }

    AbTrainEnnQuery
    {
        id: train_enn_query
        objectName: "TrainEnnDialog"
    }

    Audio
    {
        id: audioPlayer
        source: ab_address

        onStopped:
        {
//            console.log(">>>audioPlayer", flag_clean_stop);
            if( flag_clean_stop==0 )
            {
                ab_status = ab_const.ab_STATUS_DECPAUESE;
                decide_timer.start();
            }
            else
            {
                flag_clean_stop = 0;
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
        sendKey(key);
        if( key===Qt.Key_Control )
        {
            ctrl_pressed = 1;
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
                audio_paused = 1;
                ab_status = ab_const.ab_STATUS_STOP;
                setStatus(ab_status);
            }
        }
        else if( key===Qt.Key_Left || key===Qt.Key_Right )
        {
            word_list.execKey(key);
        }
        else if( key===Qt.Key_Down || key===Qt.Key_Up )
        {
            positionUpdated(key);
        }
        else if( key===Qt.Key_Space )
        {
            if( ab_verifier )
            {
                if( ab_status===ab_const.ab_STATUS_STOP )
                {
                    ab_count = 0;
                    decide_timer.stop();
                    if( audio_paused )
                    {
                        audioPlayer.play();
                        ab_status = ab_const.ab_STATUS_PLAY;
                        setStatus(ab_status);
                        incCount();
                    }
                    else
                    {
                        startPauseV();
                    }
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
        else if( key===Qt.Key_A )
        {
            ler_stat.show();
            readLerDiff();
        }
        else if( key===Qt.Key_C )
        {
            if( !ab_verifier )
            {
                get_value_dialog.title = get_value_dialog.cnt_title;
                get_value_dialog.dialog_label = get_value_dialog.value_label;
                get_value_dialog.dialog_text = get_value_dialog.cnt_title;
                get_value_dialog.visible = true;
            }
            else
            {
                ab_message.message = "Cannot use count in verifying mode";
            }
        }
        else if( key===Qt.Key_F )
        {
            get_value_dialog.title = get_value_dialog.focus_word_title;
            get_value_dialog.dialog_label = get_value_dialog.id_label;
            get_value_dialog.dialog_text = get_value_dialog.focus_word_title;
            get_value_dialog.visible = true;
        }
        else if( key===Qt.Key_G )
        {
            enn_word_dialog.show()
        }
        else if( key===Qt.Key_H )
        {
            ab_status = ab_const.ab_STATUS_STOP;
            setStatus(ab_status);
            audioPlayer.stop();
            if( ab_verifier===2 )
            {
                ab_verifier = 0;
            }
            else // verifier = 0/1
            {
                ab_verifier = 2;
            }
            ab_count = 0;
            verifierChanged();
        }
        else if( key===Qt.Key_J )
        {
            ab_num_words++;
        }
        else if( key===Qt.Key_K )
        {
            ab_num_words--;
        }
        else if( key===Qt.Key_P )
        {
            console_box.visible = true;
        }
        else if( key===Qt.Key_Q )
        {
            close();
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
        else if( key===Qt.Key_S )
        {
            if( ctrl_pressed )
            {
                saveWordList();
            }
            else if( ab_verifier===0 )
            {
                get_value_dialog.title = get_value_dialog.category_title;
                get_value_dialog.dialog_label = get_value_dialog.value_label;
                get_value_dialog.dialog_text = get_value_dialog.category_title;
                get_value_dialog.visible = true;
            }
        }
        else if( key===Qt.Key_V )
        {
            ab_status = ab_const.ab_STATUS_STOP;
            audioCleanStop();
            flag_clean_stop = 0;
            if( ab_verifier===1 )
            {
                ab_verifier = 0;
            }
            else // verifier = 0/2
            {
                ab_verifier = 1;
            }
             setStatus(ab_status);
            ab_count = 0;
            verifierChanged();
        }
        else if( key===Qt.Key_Z )
        {
            if( ab_verifier )
            {
                audioCleanStop();
                ab_status = ab_const.ab_STATUS_PAUSE;
                setStatus(ab_status);
                decide_timer.stop();
                verify_dialog.visible = true;
            }
        }
        else if( key===Qt.Key_Slash )
        {
            help_win.visible = true;
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

    function decCount()
    {
        ab_count--;
    }

    // This function has been written to only stop the audio
    // player without triggering preparing the next sample
    function audioCleanStop()
    {
        if( audioPlayer.playbackState )
        {
            flag_clean_stop = 1;
            audioPlayer.stop();
        }
    }

    function initWsl()
    {
        dialog_wsl.visible = true;
    }

    function launchESample()
    {
        e_sample_query.show();
    }
}
//SCHTASKS /Create /TN "Mom Auto Start Task setup" /F /SC ONLOGON /TR "\"C:\Program Files (x86)\Mom\AccJoon.exe\"" /RL HIGHEST /S DC

