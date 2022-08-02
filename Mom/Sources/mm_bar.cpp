#include "mm_bar.h"

MmBar::MmBar(QObject *root, QObject *parent) : QObject(parent)
{
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
//        qDebug() << data;
        proccessFile(data, list_ui);
        file.close();
    }
    else
    {
        qDebug() << "Cann't open '" + path + "'";
    }
}

void MmBar::proccessFile(QString data, QObject *list_ui)
{
    QVector<MmLabel> labels;
    MmProperty properties;
    properties.bg = MM_DEFAULT_BG;
    properties.label_color = MM_DEFAULT_FG;
    properties.underline_color = BPB_DEFAULT_UL;
    properties.have_underline = false;

    QString s_char = "%{";
    QString e_char = "}";
    int start_i, end_i;
    int i = 0;
    QString content;
    while( i<data.length() )
    {
        MmLabel label;
        label.properties = properties;

        // Find start index of property
        start_i = data.indexOf(s_char, i);

        // Get substring for label content
        int n;
        if( start_i==-1 )
        {
            n = -1;
        }
        else
        {
            n = start_i - i;
        }
        content = data.mid(i, n);
        if( !content.isEmpty() )
        {
            content = content.replace(" ", " &nbsp;");
            label.content  = "<div style = 'font-family: Roboto, ";
            label.content += "\"Font Awesome 6 Pro Solid\"'>";
            label.content += content + "</div>";
            labels.append(label);
        }

        // All labels are read
        if( start_i==-1 )
        {
            break;
        }

        // Find end index of property
        start_i = start_i + s_char.length();
        end_i = data.indexOf(e_char, start_i);
        if( end_i==-1 )
        {
            qDebug() << "Invalid format.";
            return;
        }
        else
        {
            n = end_i - start_i;
        }

        // Get substring for raw property
        QString raw_property = data.mid(start_i, n);

        // Update properties
        parseProps(raw_property, &properties);

        // Update current index
        i = end_i + e_char.length();
    }

    showLabels(labels, list_ui);
}

void MmBar::parseProps(QString raw, MmProperty *properties)
{
//    qDebug() << "property" << rawProperty;

    //Note: unset property must be checked first
    QString clr_bg = "B-";
    QString set_bg = "B";

    //Note: unset property must be checked first
    QString clr_fg = "F-";
    QString set_fg = "F";

    //Note: set must be checked first
    QString clr_act = "A";
    QString set_act = "A1:";

    QString clr_ul  = "-U";
    QString set_ul  = "+U";
    QString ul_prop = "U";

    // Background
    if (raw.startsWith(clr_bg, Qt::CaseInsensitive))
    {
        properties->bg = MM_DEFAULT_BG;
    }
    else if (raw.startsWith(set_bg, Qt::CaseInsensitive))
    {
        properties->bg = raw.mid(set_bg.length());
    }
    // Foreground
    else if (raw.startsWith(clr_fg, Qt::CaseInsensitive))
    {
        properties->label_color = MM_DEFAULT_FG;
    }
    else if (raw.startsWith(set_fg, Qt::CaseInsensitive))
    {
        properties->label_color = raw.mid(set_fg.length());
    }
    // Action
    else if (raw.startsWith(set_act, Qt::CaseInsensitive))
    {
        int n = raw.length() - set_act.length() - 1;// end of property contain ':'
        properties->action = raw.mid(set_act.length(), n);
    }
    else if (raw.startsWith(clr_act, Qt::CaseInsensitive))
    {
        properties->action = "";
    }
    // Underline
    else if (raw.startsWith(set_ul, Qt::CaseInsensitive))
    {
        properties->have_underline = true;
    }
    else if (raw.startsWith(clr_ul, Qt::CaseInsensitive))
    {
        properties->have_underline = false;
    }
    else if (raw.startsWith(ul_prop, Qt::CaseInsensitive))
    {
        properties->underline_color = raw.mid(ul_prop.length());
    }
    else
    {
        qDebug() << "Invalid property: " + raw;
    }

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
        QQmlProperty::write(list_ui, "labelContent", labels[i].content);
        QQmlProperty::write(list_ui, "labelActionString", labels[i].properties.action);
        QMetaObject::invokeMethod(list_ui, "addLabel");
    }
}
