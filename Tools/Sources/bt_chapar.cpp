#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
    state = new ReState;
    channel = new ReChannelL;

    connect(state, SIGNAL(updateMode()), this, SLOT(updateMode()));
}

void ReChapar::updateMode()
{
#ifdef _WIN32
    RePage c_page; // current page
    if( state->getMode()==bt_MODE_MAIN )
    {
        c_page.x_action = "Close Active Application";
        c_page.y_action = "Switch Application";
        c_page.a_action = "Open Application";
        c_page.b_action = "Control Music";
        c_page.s_action = "Open New Firefox Window";

        c_page.r1_action = getShortTitle(0);
        c_page.r2_action = getShortTitle(1);
        c_page.l1_action = getShortTitle(2);
        c_page.l2_action = getShortTitle(3);
        c_page.m_action = "Close Super Mode";

        c_page.left_action = "Switch to Workspace #1";
        c_page.up_action = "Switch to Workspace #2";
        c_page.down_action = "Switch to Workspace #3";
        c_page.right_action = "Switch to Workspace #4";

        c_page.axis_state = "1";
    }
    else if( state->getMode()==bt_MODE_APPLICATION )
    {
        c_page.x_action = "Open Firefox";
        c_page.y_action = "Open Spotify";
        c_page.a_action = "Open Telegram";
        c_page.b_action = "Connect VPN";
        c_page.s_action = "Suspend the PC";

        c_page.r1_action = "Qt Creator";
        c_page.r2_action = "Git Kraken";
        c_page.l1_action = "Allegro 17.4";
        c_page.l2_action = "Allegro 17.2";
        c_page.m_action = "Close Super Mode";

        c_page.lau_action = "Sajad jooOon";
        c_page.lad_action = "Bijan Joon";
        c_page.lal_action = "Karim joon";
        c_page.lar_action = "Ehsan Joon";
        c_page.rau_action = "Abdi joooon";
        c_page.rad_action = "Aflatoon joon";
        c_page.ral_action = "Narges joon";
        c_page.rar_action = "Sepehr joon";
        c_page.left_action = "Switch to Workspace #1";
        c_page.up_action = "Switch to Workspace #2";
        c_page.down_action = "Switch to Workspace #3";
        c_page.right_action = "Switch to Workspace #4";
        c_page.axis_state = "0";


        if ( state->vpn_connected )
        {
            c_page.b_action = "Disconnect VPN";
        }
    }
    else if( state->getMode()==bt_MODE_SWITCH )
    {
        c_page.x_action = state->api->getWinTitle(0);
        c_page.y_action = state->api->getWinTitle(1);
        c_page.a_action = state->api->getWinTitle(2);
        c_page.b_action = state->api->getWinTitle(3);
        c_page.s_action = state->api->getWinTitle(4);

        c_page.r1_action = state->api->getWinTitle(5);
        c_page.r2_action = state->api->getWinTitle(6);
        c_page.l1_action = state->api->getWinTitle(7);
        c_page.l2_action = state->api->getWinTitle(8);

        if(thread_data->message.isEmpty())
        {
            thread_data->message = "Launch Nuclear missiles";
        }
    }
    /*if( state->getProcess()==bt_WIN_EXPLORER )
    {
        ;
    }
    else if( state->getProcess()==bt_WIN_FIREFOX )
    {
        ;
    }
    else if( state->getProcess()==bt_WIN_QT )
    {
        ;
    }
    else if( state->getProcess()==bt_WIN_READING )
    {
        ;
    }*/
    else if( state->getMode()==bt_MODE_SPOTIFY )
    {
        c_page.x_action = "Play/Pause";
        c_page.y_action = "Repeat Mode";
        c_page.a_action = "Like Song";
        c_page.b_action = "Stop Music";
        c_page.m_action = "Close Super Mode";
        c_page.s_action = "Mute/UnMute";

        c_page.r1_action = "Next Music";
        c_page.l1_action = "Prev Music";
        c_page.r2_action = "Increase Volume";
        c_page.l2_action = "Decrease Volume";

        c_page.lau_action = "-40% Volume";
        c_page.lar_action = "-30% Volume";
        c_page.lal_action = "-20% Volume";
        c_page.lad_action = "-10% Volume";
        c_page.rau_action = "+40% Volume";
        c_page.rar_action = "+30% Volume";
        c_page.ral_action = "+20% Volume";
        c_page.rad_action = "+10% Volume";

//        c_page.lau_action = state->api->getElemName(0).split(" ")[0];
//        c_page.lar_action = state->api->getElemName(1).split(" ")[0];
//        c_page.lal_action = state->api->getElemName(2).split(" ")[0];
//        c_page.lad_action = state->api->getElemName(3).split(" ")[0];
//        c_page.rau_action = state->api->getElemName(4).split(" ")[0];
//        c_page.rar_action = state->api->getElemName(5).split(" ")[0];
//        c_page.ral_action = state->api->getElemName(6).split(" ")[0];
//        c_page.rad_action = state->api->getElemName(7).split(" ")[0];
    }
    setPage(c_page);
#endif
}

void ReChapar::switchWindow(int index)
{
    //state->setMode(bt_MODE_HIDDEN);
}

void ReChapar::requstSuspend()
{
#ifdef _WIN32
    state->hardware->disconnectXbox();
#endif
}
