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
    property string labelActionL: ""
    property string labelActionR: ""
    property string labelActionM: ""
    property string labelActionU: ""
    property string labelActionD: ""
    property int    labelCount: 0
    property int    isLeft: 0

    // Cpp Signals
    signal executeAction(string m_action)

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
            label_action_l: labelActionL
            label_action_r: labelActionR
            label_action_m: labelActionM
            label_action_u: labelActionU
            label_action_d: labelActionD

            onLabelClicked: executeAction(action)
        }
    }

    function clearLabels()
    {
        for( var i=labelID ; i<lm.count ; )
        {
            lm.remove(i);
        }
        labelCount = 0;
    }

    function addLabel()
    {
        lm.append({
                      "colorBackground": labelBg,
                      "labelColor": labelFg,
                      "underlineColor": labelUl,
                      "haveUnderline": labelUlEn,
                      "labelText": labelVal,
                      "labelActionL": labelActionL,
                      "labelActionR": labelActionR,
                      "labelActionM": labelActionM,
                      "labelActionU": labelActionU,
                      "labelActionD": labelActionD
                  });
        labelCount++;
    }

    function updateLabel()
    {
        if( labelID>=lm.count )
        {
            console.log("updateLabel Error", labelID)
            return;
        }

        lm.get(labelID).colorBackground = labelBg;
        lm.get(labelID).labelColor      = labelFg;
        lm.get(labelID).underlineColor  = labelUl;
        lm.get(labelID).haveUnderline   = labelUlEn;
        lm.get(labelID).labelText   = labelVal;
        lm.get(labelID).labelActionL = labelActionL;
        lm.get(labelID).labelActionR = labelActionR;
        lm.get(labelID).labelActionM = labelActionM;
        lm.get(labelID).labelActionU = labelActionU;
        lm.get(labelID).labelActionD = labelActionD;
    }
}
