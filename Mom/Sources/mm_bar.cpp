#include "mm_bar.h"

MmBar::MmBar(QObject *root, QObject *parent) : QObject(parent)
{
    parser = new MmParser;
    // List ui
    left_bar_ui =   root->findChild<QObject*>("LeftBar");
    right_bar_ui =  root->findChild<QObject*>("RightBar");

    connect(left_bar_ui, SIGNAL(executeAction(QString)),
            this, SLOT(executeCommand(QString)));
    connect(right_bar_ui, SIGNAL(executeAction(QString)),
            this, SLOT(executeCommand(QString)));

    // Timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateLabels()));
    timer->start(MM_BAR_TIMEOUT);

    // Load labels
    loadLabels(MM_L1_LABEL, left_bar_ui);
    loadLabels(MM_R1_LABEL, right_bar_ui);
}

/***************** Private Slots *****************/
void MmBar::executeCommand(QString action)
{
    qDebug() << "execute" << action;
}

void MmBar::updateLabels()
{
    loadLabels(MM_L1_LABEL, left_bar_ui);
    loadLabels(MM_R1_LABEL, right_bar_ui);
}

void MmBar::loadLabels(QString path, QObject *list_ui)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QString data = file.readAll();
        parser->proccessFile(data);
        showLabels(parser->labels, list_ui);
        file.close();
    }
    else
    {
        qDebug() << "Cann't open '" + path + "'";
    }
}

void MmBar::proccessFile(QString data, QObject *list_ui)
{

}

void MmBar::showLabels(QVector<MmLabel> labels, QObject *list_ui)
{
    QMetaObject::invokeMethod(list_ui, "clearLabels");
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
