#include "mm_bar.h"
#include <QDir>

MmBar::MmBar(QVector<QWindow *> windows, MmVirt *vi, MmSound *snd,
             QObject *parent) : QObject(parent)
{
    wins    = windows;
    parser  = new MmParser(windows[0]);
    sound   = snd;
    usage   = new MmUsage;
    music   = new MmMusic;
    virt    = vi;

    // run namedpipe channel
    channel = new MmChannel;
    channel_thread = new QThread();
    channel->moveToThread(channel_thread);
    channel_thread->start();

    connect(this, SIGNAL(startChannel()), channel, SLOT(listenPipe()));
    connect(channel, SIGNAL(meta(QString)),
            this, SLOT(executeAction(QString)));

    emit startChannel();
    // List ui
    int win_len = windows.size();
    left_bars.resize(win_len);
    right_bars.resize(win_len);
    for( int i=0 ; i<win_len ; i++ )
    {
        left_bars[i]  = windows[i]->findChild<QObject*>("LeftBar");
        right_bars[i] = windows[i]->findChild<QObject*>("RightBar");
        connect(left_bars[i], SIGNAL(executeAction(QString)),
                this, SLOT(executeAction(QString)));
        connect(right_bars[i], SIGNAL(executeAction(QString)),
                this, SLOT(executeAction(QString)));
    }

    // Timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(loadLabels()));
    timer->start(MM_BAR_TIMEOUT);
}

void MmBar::executeAction(QString action)
{
    if( action=="sound" )
    {
        sound->leftClick();
    }
    else if( action=="vol_up" )
    {
        sound->volumeUp();
    }
    else if( action=="vol_down" )
    {
        sound->volumeDown();
    }
    else if( action=="prev" )
    {
        music->prevClick();
    }
    else if( action=="play" )
    {
        music->playClick();
    }
    else if( action=="next" )
    {
        music->nextClick();
    }
    else
    {
        qDebug() << "execute" << action;
        int desktop_id = action.toInt();
        virt->setDesktop(desktop_id-1);
    }

    /// TODO: FIXME
//    int focus = QQmlProperty::read(wins[0], "focus_back").toInt();
    int focus = 1;
    if( focus )
    {
        music->emul->altTab();
    }
}

void MmBar::loadLabels()
{
    QDir p_dir(MM_LABEL_DIR);
    QStringList fmt;
    fmt.append("*.lbl");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    //add workspace widget
    int l_id = updateWorkSpace();

    //add soundbar widget
    int r_id = addRWidget();

    for( int i=0 ; i<file_list.length() ; i++ )
    {
        QString path = MM_LABEL_DIR;
        path += file_list[i];

        if( file_list[i][0]=="l" )
        {
            l_id += proccessFile(left_bars, l_id, path);
        }
        else
        {
            r_id += proccessFile(right_bars, r_id, path);
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

    int len = left_bars.size();
    for( int i=0 ; i<len ; i++ )
    {
        QQmlProperty::write(left_bars[i], "labelID", l_id);
        QMetaObject::invokeMethod(left_bars[i], "rmSpurLabels");
        QQmlProperty::write(right_bars[i], "labelID", r_id);
        QMetaObject::invokeMethod(right_bars[i], "rmSpurLabels");

        // update bg color
        if( QFile::exists(MM_LABEL_DIR"r2_status.lbl") )
        {
            QQmlProperty::write(wins[i], "bg_color", "#0067aa");
        }
        else
        {
            QQmlProperty::write(wins[i], "bg_color", "#000000");
        }
    }
}

int MmBar::updateWorkSpace()
{
    MmLabel virt_lbl;
    int virt_idx = virt->getCurrDesktop();
    QString lbl_val = getWorkStr(virt_idx);
    QVector<MmLabel> out;
    parser->parse(lbl_val, &out);

    int len = out.length();
    for(int i=0 ; i<len ; i++ )
    {
        updateLabel(left_bars[0], i, out[i]);
    }

    return len;
}

int MmBar::addRWidget()
{
    int ret;
    MmLabel music_lbl;
    QString music_val = music->getLabel();
    QVector<MmLabel> out_music;
    parser->parse(music_val, &out_music);

    int len = out_music.length();
    for(int i=0 ; i<len ; i++ )
    {
        updateLabel(right_bars[0], i, out_music[i]);
    }
    ret = len;

    MmLabel usage_lbl;
    QString usage_val = usage->getLabel();
    QVector<MmLabel> out_usage;
    parser->parse(usage_val, &out_usage);

    len = out_usage.length();
    for(int i=0 ; i<len ; i++ )
    {
        updateLabel(right_bars[0], i+ret, out_usage[i]);
    }
    ret += len;

    MmLabel sound_lbl;
    QString sound_val = sound->getLabel();
    QVector<MmLabel> out_sound;
    parser->parse(sound_val, &out_sound);

    len = out_sound.length();
    for(int i=0 ; i<len ; i++ )
    {
        updateLabel(right_bars[0], i+ret, out_sound[i]);
    }
    ret += len;

    return ret;
}

QString MmBar::getWorkStr(int index)
{
    QString ret;
    index--;

    QStringList tag_list;

    tag_list << "%{A1:1:}   %{A1}";
    tag_list << "%{A1:2:}   %{A1}";
    tag_list << "%{A1:3:}   %{A1}";
    tag_list << "%{A1:4:}    %{A1}";
    tag_list << "%{A1:5:}    %{A1}";
    tag_list << "%{A1:6:}    %{A1}";

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

int MmBar::proccessFile(QVector<QObject *>bars, int s_id,
                        QString path)
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
        updateLabel(bars[0], i, out[i-s_id]);
    }

    file.close();

    return len;
}

void MmBar::addLabel(QObject *bar, MmLabel label)
{
    QVector<QObject *> bars;
    int is_left = QQmlProperty::read(bar, "isLeft").toInt();
    if( is_left )
    {
        l_labels.append(label);
        bars = left_bars;
    }
    else
    {
        r_labels.append(label);
        bars = right_bars;
    }

    int len_bars = bars.size();
    for( int i=0 ; i<len_bars ; i++ )
    {
        QQmlProperty::write(bars[i], "labelBg", label.prop.bg);
        QQmlProperty::write(bars[i], "labelFg", label.prop.fg);
        QQmlProperty::write(bars[i], "labelUl", label.prop.ul);
        QQmlProperty::write(bars[i], "labelUlEn", label.prop.ul_en);
        QQmlProperty::write(bars[i], "labelVal", label.val);
        QQmlProperty::write(bars[i], "labelActionL", label.prop.action_l);
        QQmlProperty::write(bars[i], "labelActionR", label.prop.action_r);
        QQmlProperty::write(bars[i], "labelActionM", label.prop.action_m);
        QQmlProperty::write(bars[i], "labelActionU", label.prop.action_u);
        QQmlProperty::write(bars[i], "labelActionD", label.prop.action_d);
        QMetaObject::invokeMethod(bars[i], "addLabel");
    }
}

void MmBar::updateUILabel(QObject *bar, int id, MmLabel label)
{
    QVector<MmLabel> *c_labels;
    QVector<QObject *> bars;
    int is_left = QQmlProperty::read(bar, "isLeft").toInt();
    if( is_left )
    {
        c_labels = &l_labels;
        bars = left_bars;
    }
    else
    {
        c_labels = &r_labels;
        bars = right_bars;
    }
    (*c_labels)[id].val = label.val;
    (*c_labels)[id].prop.bg = label.prop.bg;
    (*c_labels)[id].prop.fg = label.prop.fg;
    (*c_labels)[id].prop.ul = label.prop.ul;
    (*c_labels)[id].prop.ul_en = label.prop.ul_en;
    (*c_labels)[id].prop.action_l = label.prop.action_l;
    (*c_labels)[id].prop.action_r = label.prop.action_r;
    (*c_labels)[id].prop.action_m = label.prop.action_m;
    (*c_labels)[id].prop.action_u = label.prop.action_u;
    (*c_labels)[id].prop.action_d = label.prop.action_d;

    int len_bars = bars.size();
    for( int i=0 ; i<len_bars ; i++ )
    {
        QQmlProperty::write(bars[i], "labelID", id);
        QQmlProperty::write(bars[i], "labelBg", label.prop.bg);
        QQmlProperty::write(bars[i], "labelFg", label.prop.fg);
        QQmlProperty::write(bars[i], "labelUl", label.prop.ul);
        QQmlProperty::write(bars[i], "labelUlEn", label.prop.ul_en);
        QQmlProperty::write(bars[i], "labelVal", label.val);
        QQmlProperty::write(bars[i], "labelActionL", label.prop.action_l);
        QQmlProperty::write(bars[i], "labelActionR", label.prop.action_r);
        QQmlProperty::write(bars[i], "labelActionM", label.prop.action_m);
        QQmlProperty::write(bars[i], "labelActionU", label.prop.action_u);
        QQmlProperty::write(bars[i], "labelActionD", label.prop.action_d);
        QMetaObject::invokeMethod(bars[i], "updateLabel");
    }
}

void MmBar::updateLabel(QObject *bar, int index, MmLabel new_lbl)
{
    int count = QQmlProperty::read(bar, "labelCount").toInt();
    int is_left = QQmlProperty::read(bar, "isLeft").toInt();

    if( index>=count )
    {
        addLabel(bar, new_lbl);
        return;
    }

    MmLabel *curr_label;
    if( is_left )
    {
        curr_label = &(l_labels[index]);
    }
    else
    {
       curr_label = &(r_labels[index]);
    }

    if( isChanged(curr_label, &new_lbl) )
    {
        updateUILabel(bar, index, new_lbl);
    }
}

int MmBar::isChanged(MmLabel *label1, MmLabel *label2)
{
    if( label1->val!=label2->val )
    {
        return 1;
    }

    if( label1->prop.bg!=label2->prop.bg )
    {
        return 1;
    }

    if( label1->prop.fg!=label2->prop.fg )
    {
        return 1;
    }

    if( label1->prop.ul!=label2->prop.ul )
    {
        return 1;
    }

    if( label1->prop.action_l!=label2->prop.action_l )
    {
        return 1;
    }

    if( label1->prop.action_r!=label2->prop.action_r )
    {
        return 1;
    }

    if( label1->prop.action_m!=label2->prop.action_m )
    {
        return 1;
    }

    if( label1->prop.action_u!=label2->prop.action_u )
    {
        return 1;
    }

    if( label1->prop.action_d!=label2->prop.action_d )
    {
        return 1;
    }

    return 0;
}
