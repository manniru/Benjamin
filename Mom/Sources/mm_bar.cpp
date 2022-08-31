#include "mm_bar.h"
#include <QDir>

MmBar::MmBar(QObject *root, MmVirt *vi,
             QObject *parent) : QObject(parent)
{
    parser = new MmParser;
    virt   = vi;
    // List ui
    left_bar  = root->findChild<QObject*>("LeftBar");
    right_bar = root->findChild<QObject*>("RightBar");

    connect(left_bar, SIGNAL(executeAction(QString)),
            this, SLOT(executeCommand(QString)));
    connect(right_bar, SIGNAL(executeAction(QString)),
            this, SLOT(executeCommand(QString)));

    // Timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(loadLabels()));
    timer->start(MM_BAR_TIMEOUT);
}

void MmBar::executeCommand(QString action)
{
    qDebug() << "execute" << action;
    int desktop_id = action.toInt();
    virt->setDesktop(desktop_id-1);
}

void MmBar::loadLabels()
{
    QDir p_dir(MM_LABEL_DIR);
    QStringList fmt;
    fmt.append("*.lbl");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    l_id = 0;
    r_id = 0;
    addWorkID();

    for( int i=0 ; i<file_list.length() ; i++ )
    {
        QString path = MM_LABEL_DIR;
        path += file_list[i];

        if( file_list[i][0]=="l" )
        {
            side = left_bar;
            l_id += proccessFile(l_id, path);
        }
        else
        {
            side = right_bar;
            r_id += proccessFile(r_id, path);
        }
    }

    for( int i=l_id ; i<l_labels.length() ; )
    {
        l_labels.remove(i);
    }

    for( int i=r_id ; i<r_labels.length() ; )
    {
        r_labels.remove(i);
    }

    QQmlProperty::write(left_bar, "labelID", l_id);
    QMetaObject::invokeMethod(left_bar, "clearLabels");
    QQmlProperty::write(right_bar, "labelID", r_id);
    QMetaObject::invokeMethod(right_bar, "clearLabels");
}

void MmBar::addWorkID()
{
    MmLabel virt_lbl;
    int virt_idx = virt->getCurrDesktop();
    QString lbl_val = getWorkStr(virt_idx);
    QVector<MmLabel> out;
    parser->parse(lbl_val, &out);

    int len = out.length();
    side = left_bar;
    for(int i=0 ; i<len ; i++ )
    {
        updateLbl(i, out[i]);
    }

    l_id += len;
}

QString MmBar::getWorkStr(int index)
{
    QString ret;
    index--;

    QStringList tag_list;

    tag_list << "%{A1:1:}   %{A}";
    tag_list << "%{A1:2:}   %{A}";
    tag_list << "%{A1:3:}   %{A}";
    tag_list << "%{A1:4:}    %{A}";
    tag_list << "%{A1:5:}    %{A}";
    tag_list << "%{A1:6:}    %{A}";

    QString p_format = "%{B#555555}%{F#f3c84a}";

    for( int i=0 ; i<index ; i++ )
    {
        ret += tag_list[i];
    }

    if( index<tag_list.count() )
    {
        ret += p_format;
        ret += tag_list[index];
        ret += "%{B-}%{F-}";
    }

    for( int i=index+1 ; i<tag_list.count() ; i++ )
    {
        ret += tag_list[i];
    }
    ret += "  ";

    return ret;
}

int MmBar::proccessFile(int s_id, QString path)
{
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Cannot open '" + path + "'";
        return 0;
    }
    QString data = file.readAll();
    QVector<MmLabel> out;
    parser->parse(data, &out);

    int len = out.length();
    for(int i=s_id ; i<s_id+len ; i++ )
    {
        updateLbl(i, out[i-s_id]);
    }

    file.close();

    return len;
}

void MmBar::addLabel(MmLabel labels)
{
    QQmlProperty::write(side, "labelBg", labels.prop.bg);
    QQmlProperty::write(side, "labelFg", labels.prop.fg);
    QQmlProperty::write(side, "labelUl", labels.prop.ul);
    QQmlProperty::write(side, "labelUlEn", labels.prop.ul_en);
    QQmlProperty::write(side, "labelVal", labels.val);
    QQmlProperty::write(side, "labelAction", labels.prop.action);
    QMetaObject::invokeMethod(side, "addLabel");

    if( side==left_bar )
    {
        l_labels.append(labels);
    }
    else
    {
        r_labels.append(labels);
    }
}

void MmBar::updateLabel(int id, MmLabel labels)
{
    QQmlProperty::write(side, "labelID", id);
    QQmlProperty::write(side, "labelBg", labels.prop.bg);
    QQmlProperty::write(side, "labelFg", labels.prop.fg);
    QQmlProperty::write(side, "labelUl", labels.prop.ul);
    QQmlProperty::write(side, "labelUlEn", labels.prop.ul_en);
    QQmlProperty::write(side, "labelVal", labels.val);
    QQmlProperty::write(side, "labelAction", labels.prop.action);
    QMetaObject::invokeMethod(side, "updateLabel");

    QVector<MmLabel> *c_labels;
    if( side==left_bar )
    {
        c_labels = &l_labels;
    }
    else
    {
        c_labels = &r_labels;
    }
    (*c_labels)[id].val = labels.val;
    (*c_labels)[id].prop.bg = labels.prop.bg;
    (*c_labels)[id].prop.fg = labels.prop.fg;
    (*c_labels)[id].prop.ul = labels.prop.ul;
    (*c_labels)[id].prop.ul_en = labels.prop.ul_en;
    (*c_labels)[id].prop.action = labels.prop.action;
}

void MmBar::updateLbl(int id, MmLabel new_lbl)
{
    QVector<MmLabel> *c_labels;
    if( side==left_bar )
    {
        c_labels = &l_labels;
    }
    else
    {
        c_labels = &r_labels;
    }

    if( id>=c_labels->length() )
    {
        addLabel(new_lbl);
        return;
    }

    if( (*c_labels)[id].val!=new_lbl.val )
    {
        updateLabel(id, new_lbl);
        return;
    }

    if( (*c_labels)[id].prop.bg!=new_lbl.prop.bg )
    {
        updateLabel(id, new_lbl);
        return;
    }

    if( (*c_labels)[id].prop.fg!=new_lbl.prop.fg )
    {
        updateLabel(id, new_lbl);
        return;
    }

    if( (*c_labels)[id].prop.ul!=new_lbl.prop.ul )
    {
        updateLabel(id, new_lbl);
        return;
    }

    if( (*c_labels)[id].prop.action!=new_lbl.prop.action )
    {
        updateLabel(id, new_lbl);
        return;
    }
}
