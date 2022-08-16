#include "mm_parser.h"

#define MM_PROP_START "%{"
#define MM_PROP_END   "}"

MmParser::MmParser()
{

}

int MmParser::parseProps(QString data, int s_index)
{
    // Find end index oLf property
    int end_i = data.indexOf(MM_PROP_END, s_index);
    s_index += strlen(MM_PROP_START);
    int len = end_i - s_index;

    // Get substring for raw property
    QString prop = data.mid(s_index, len);
    updateProps(prop, &c_property);

    return end_i + strlen(MM_PROP_END);
}

void MmParser::parse(QString data, QVector<MmLabel> *out)
{
    out->clear();
    c_property.bg = MM_DEFAULT_BG;
    c_property.fg = MM_DEFAULT_FG;
    c_property.ul = BPB_DEFAULT_UL;

    int i = 0;
    while( i<data.length() )
    {
        MmLabel label;
        label.prop = c_property;

        // First index starting from i
        int start_i = data.indexOf(MM_PROP_START, i);

        QString content;
        if( start_i==-1 )
        {
            content = data.mid(i); // read to the end
        }
        else
        {
            int len = start_i - i;
            content = data.mid(i, len);

            i = parseProps(data, start_i);
        }
        if( !content.isEmpty() )
        {
            label.setVal(content);
            out->append(label);
        }

        // All labels are read
        if( start_i==-1 )
        {
            break;
        }
    }
}

void MmParser::updateProps(QString raw, MmProperty *properties)
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
        properties->fg = MM_DEFAULT_FG;
    }
    else if (raw.startsWith(set_fg, Qt::CaseInsensitive))
    {
        properties->fg = raw.mid(set_fg.length());
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
        properties->ul_en = true;
    }
    else if (raw.startsWith(clr_ul, Qt::CaseInsensitive))
    {
        properties->ul_en = false;
    }
    else if (raw.startsWith(ul_prop, Qt::CaseInsensitive))
    {
        properties->ul = raw.mid(ul_prop.length());
    }
    else
    {
        qDebug() << "Invalid property: " + raw;
    }

}
