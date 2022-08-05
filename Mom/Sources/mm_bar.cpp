#include "mm_bar.h"
#include <QDir>

MmBar::MmBar(QObject *root, QObject *parent) : QObject(parent)
{
    parser = new MmParser;
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

void MmBar::proccessFile(QString path, QObject *obj)
{
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Cann't open '" + path + "'";
        return;
    }
    QString data = file.readAll();
    parser->proccessFile(data);
    showLabels(parser->labels, obj);

    file.close();
}

void MmBar::showLabels(QVector<MmLabel> labels, QObject *list_ui)
{
    int len=labels.length();

    for(int i=0 ; i<len ; i++ )
    {
        QQmlProperty::write(list_ui, "labelBackgroundColor", labels[i].properties.bg);
        QQmlProperty::write(list_ui, "labelTextColor", labels[i].properties.label_color);
        QQmlProperty::write(list_ui, "labelUnderlineColor", labels[i].properties.underline_color);
        QQmlProperty::write(list_ui, "labelHaveUnderline", labels[i].properties.have_underline);
        QQmlProperty::write(list_ui, "labelContent", labels[i].val);
        QQmlProperty::write(list_ui, "labelActionString", labels[i].properties.action);
        QMetaObject::invokeMethod(list_ui, "addLabel");
    }
}
