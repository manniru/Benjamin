import QtQuick 2.0

Flickable
{
    // Set this variables in cpp
    property string labelID: "" //Id
    property string labelBg: "" //Background
    property string labelFg: "" //Foreground
    property string labelUl: "" //Underline Color
    property bool   labelUlEn: false //Underline Enable
    property string labelVal: ""
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
        for( var i=labelID ; i<lm.count ; )
        {
            lm.remove(i);
        }
    }

    function addLabel()
    {
        lm.append({
                      "colorBackground": labelBg,
                      "labelColor": labelFg,
                      "underlineColor": labelUl,
                      "haveUnderline": labelUlEn,
                      "labelText": labelVal,
                      "labelAction": labelAction
                  });
    }

    function updateLabel()
    {
        lm.get(labelID).colorBackground = labelBg;
        lm.get(labelID).labelColor      = labelFg;
        lm.get(labelID).underlineColor  = labelUl;
        lm.get(labelID).haveUnderline   = labelUlEn;
        lm.get(labelID).labelText   = labelVal;
        lm.get(labelID).labelAction = labelAction;
    }
}
