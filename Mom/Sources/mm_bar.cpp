#include "mm_bar.h"
#include <QDir>

MmBar::MmBar(QObject *root, QObject *parent) : QObject(parent)
{
    parser = new MmParser;
    virt   = new MmVirt;
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
}

void MmBar::loadLabels()
{
    QDir p_dir(MM_LABEL_DIR);
    QStringList fmt;
    fmt.append("*.lbl");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    QMetaObject::invokeMethod(left_bar, "clearLabels");
    QMetaObject::invokeMethod(right_bar, "clearLabels");
    addWorkID();

    for( int i=0 ; i<file_list.length() ; i++ )
    {
        QString path = MM_LABEL_DIR;
        path += file_list[i];

        if( file_list[i][0]=="l" )
        {
            proccessFile(path, left_bar);
        }
        else
        {
            proccessFile(path, right_bar);
        }
    }
}

void MmBar::addWorkID()
{
    MmLabel virt_lbl;
    int virt_idx = virt->getCurrDesktop();
    QString lbl_val = getWorkStr(virt_idx);
    QVector<MmLabel> out;
    parser->parse(lbl_val, &out);

    int len = out.length();
    for(int i=0 ; i<len ; i++ )
    {
        addLabel(out[i], left_bar);
    }
}

QString MmBar::getWorkStr(int index)
{
    QString ret;
    index--;

    QStringList tag_list;

    tag_list << "   ";
    tag_list << "   ";
    tag_list << "   ";
    tag_list << "    ";
    tag_list << "    ";
    tag_list << "    ";

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

void MmBar::proccessFile(QString path, QObject *obj)
{
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Cann't open '" + path + "'";
        return;
    }
    QString data = file.readAll();
    QVector<MmLabel> out;
    parser->parse(data, &out);

    int len = out.length();
    for(int i=0 ; i<len ; i++ )
    {
        addLabel(out[i], obj);
    }

    file.close();
}

void MmBar::addLabel(MmLabel labels, QObject *list_ui)
{
    QQmlProperty::write(list_ui, "labelBg", labels.prop.bg);
    QQmlProperty::write(list_ui, "labelFg", labels.prop.fg);
    QQmlProperty::write(list_ui, "labelUl", labels.prop.ul);
    QQmlProperty::write(list_ui, "labelUlEn", labels.prop.ul_en);
    QQmlProperty::write(list_ui, "labelContent", labels.val);
    QQmlProperty::write(list_ui, "labelAction", labels.prop.action);
    QMetaObject::invokeMethod(list_ui, "addLabel");
}
