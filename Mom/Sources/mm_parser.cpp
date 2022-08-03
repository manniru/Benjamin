#include "mm_parser.h"

MmParser::MmParser()
{

}

void MmParser::proccessFile(QString data)
{
    labels.clear();
    MmProperty c_property;
    c_property.bg = MM_DEFAULT_BG;
    c_property.label_color = MM_DEFAULT_FG;
    c_property.underline_color = BPB_DEFAULT_UL;
    c_property.have_underline = false;

    QString s_char = "%{";
    QString e_char = "}";
    int start_i, end_i;
    int i = 0;
    QString content;
    while( i<data.length() )
    {
        MmLabel label;
        label.properties = c_property;

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
            label.setVal(content);
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
        parseProps(raw_property, &c_property);

        // Update current index
        i = end_i + e_char.length();
    }
}

void MmParser::parseProps(QString raw, MmProperty *properties)
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
