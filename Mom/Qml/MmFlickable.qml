import QtQuick 2.0

Flickable
{
    // Set this variables in cpp
    property string labelBg: "" //Background
    property string labelFg: "" //Foreground
    property string labelUl: "" //Underline Color
    property bool   labelUlEn: false //Underline Enable
    property string labelContent: ""
    property string labelAction: ""

    // Cpp Signals
    signal executeAction(string action)

    width: lv.width
    clip: true

    ListView
    {
        id: lv
        height: parent.height
        width: contentWidth
        anchors.topMargin: 15
        orientation: ListView.Horizontal
        interactive: contentWidth>width
        model: ListModel
        {
            id: lm
        }
        delegate: MmLabel
        {
            height: parent.height
            anchors.top: parent.top
            color_background: colorBackground
            color_label: labelColor
            color_underline: underlineColor
            underline: haveUnderline
            label_text: labelText
            label_action: labelAction

            onLabelClicked: executeAction(labelAction)
        }
    }

    function clearLabels()
    {
        lm.clear()
    }

    function addLabel()
    {
        lm.append({
                      "colorBackground": labelBg,
                      "labelColor": labelFg,
                      "underlineColor": labelUl,
                      "haveUnderline": labelUlEn,
                      "labelText": labelContent,
                      "labelAction": labelAction
                  })
    }
}
