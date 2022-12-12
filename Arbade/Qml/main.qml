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
    property int window_base_width : 1080
    property int window_base_height: 600

    width: window_base_width
    height: window_base_height
    minimumHeight: 500
    minimumWidth: 760

    color: "#FFFFFF"
    title: "ArBade"
    visible: true

    AbHelp
    {
        id: ab_help

        width: root.width/2
        height: root.height
        anchors.top: parent.top
        anchors.right: parent.right
        key_text: "Help\n" +
                   "Up:\n" +
                   "Down:\n" +
                   "Right:\n" +
                   "Left:\n" +
                   "K:\n" +
                   "J:\n" +
                   "Space:\n" +
                   "Q:\n" +
                   "Enter:\n" +
                   "T:\n" +
                   "C:"
        description_text: "\n" +
                          "Increase Pause\n" +
                          "Decrease Pause\n" +
                          "Increase Num of Words\n" +
                          "Decrease Num of Words\n" +
                          "Increase Rec Time\n" +
                          "Decrease Rec Time\n" +
                          "Pause Recording\n" +
                          "Close Window\n" +
                          "Change Category name\n" +
                          "Open Category Directory\n" +
                          "Change Count Number"
    }

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

        Keys.onPressed:
        {
            execKey(event.key)
        }
    }

    AbStatus
    {
        width: root.width/2
        height: ab_help.text_heigh
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        pause_time: root_scene.pausetime
        num_words: root_scene.numwords
        rec_time: root_scene.rectime
        status: root_scene.status
        words: root_scene.words
        category: root_scene.category
        count: root_scene.count
        count_total: root_scene.totalcount
        elapsed_time: root_scene.elapsedtime
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
                root_scene.category = get_value_dialog.dialog_text
            }
            else if( title==="Enter Count" )
            {
                root_scene.totalcount = parseInt(get_value_dialog.dialog_text)
            }
        }
    }

    Component.onCompleted:
    {
        root_scene.qmlcreated = 1
        updatePlotSize()
    }

    //Fonts:
    FontLoader
    {
        id: fontRobotoRegular
        source: "qrc:/Roboto-Regular.ttf"
    }

    function updatePlotSize()
    {

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
            if( root_scene.status===ab_const.ab_STATUS_REC ||
                root_scene.status===ab_const.ab_STATUS_BREAK )
            {
                root_scene.status = ab_const.ab_STATUS_REQPAUSE
            }
            else if( root_scene.status===ab_const.ab_STATUS_PAUSE ||
                     root_scene.status===ab_const.ab_STATUS_STOP )
            {
                root_scene.status = ab_const.ab_STATUS_REC
            }
        }
        else if( key===Qt.Key_Q )
        {
            close()
        }
        else if( key===Qt.Key_Return )
        {
            get_value_dialog.title = "Enter Category"
            get_value_dialog.open()
        }
        else if( key===Qt.Key_T )
        {

        }
        else if( key===Qt.Key_C )
        {
            get_value_dialog.title = "Enter Count"
            get_value_dialog.open()
        }
    }
}
