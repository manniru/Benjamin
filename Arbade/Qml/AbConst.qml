import QtQuick 2.0

Item
{
    property int ab_STATUS_REC:       1
    property int ab_STATUS_PAUSE:     2
    property int ab_STATUS_STOP:      3
    property int ab_STATUS_REQPAUSE:  4
    property int ab_STATUS_BREAK:     5
    property int ab_STATUS_PLAY:      6
    property int ab_STATUS_DECPAUESE: 7 // Decide Pause

    property int ab_DECIDE_PAUSE:      root.ab_verify_pause*1000*3
}
