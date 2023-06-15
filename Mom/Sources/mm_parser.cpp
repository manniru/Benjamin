#include "mm_parser.h"

#define MM_PROP_START "%{"
#define MM_PROP_END   "}"

MmParser::MmParser(QObject *root)
{
    ui     = root;
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
    QString ui_bg = QQmlProperty::read(ui, "bg_color").toString();
    c_property.bg = ui_bg;
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
    // Example: A1:sound:
//    qDebug() << "property" << raw;

    //Note: unset property must be checked first
    QString bg_set = "B";
    QString bg_clr = "B-";

    //Note: unset property must be checked first
    QString fg_set = "F";
    QString fg_clr = "F-";

    QString ul_clr  = "-U";
    QString ul_set  = "+U";
    QString prop_ul = "U";

    // Background
    if( raw.startsWith(bg_clr) )
    {
        QString ui_bg = QQmlProperty::read(ui, "bg_color").toString();
        properties->bg = ui_bg;
    }
    else if( raw.startsWith(bg_set) )
    {
        properties->bg = raw.mid(bg_set.length());
    }
    // Foreground
    else if( raw.startsWith(fg_clr) )
    {
        properties->fg = MM_DEFAULT_FG;
    }
    else if( raw.startsWith(fg_set) )
    {
        properties->fg = raw.mid(fg_set.length());
    }
    // Action
    else if( readActions(raw, properties) )
    {
        return;
    }
    // Underline
    else if( raw.startsWith(ul_set) )
    {
        properties->ul_en = true;
    }
    else if( raw.startsWith(ul_clr) )
    {
        properties->ul_en = false;
    }
    else if( raw.startsWith(prop_ul) )
    {
        properties->ul = raw.mid(prop_ul.length());
    }
    else
    {
        qDebug() << "Invalid property: " + raw;
    }

}

int MmParser::readActions(QString raw, MmProperty *properties)
{
    //Note: clear must be checked first
    QString act_l_set = "A1:";
    QString act_l_clr = "A1";

    QString act_r_set = "A2:";
    QString act_r_clr = "A2";

    QString act_m_set = "A3:";
    QString act_m_clr = "A3";

    QString act_u_set = "A4:";
    QString act_u_clr = "A4";

    QString act_d_set = "A5:";
    QString act_d_clr = "A5";

    if( raw==act_l_clr )
    {
        properties->action_l = "";

        return 1;
    }
    else if( raw.startsWith(act_l_set) )
    {
        // end of property contain ':'(1)
        int len = raw.length() - act_l_set.length() - 1;
        properties->action_l = raw.mid(act_l_set.length(), len);

        return 1;
    }

    else if( raw==act_r_clr )
    {
        properties->action_r = "";

        return 1;
    }
    else if( raw.startsWith(act_r_set) )
    {
        // end of property contain ':'(1)
        int len = raw.length() - act_r_set.length() - 1;
        properties->action_r = raw.mid(act_r_set.length(), len);

        return 1;
    }

    else if( raw==act_m_clr )
    {
        properties->action_m = "";

        return 1;
    }
    else if( raw.startsWith(act_m_set) )
    {
        // end of property contain ':'(1)
        int len = raw.length() - act_m_set.length() - 1;
        properties->action_m = raw.mid(act_m_set.length(), len);

        return 1;
    }

    else if( raw==act_u_clr )
    {
        properties->action_u = "";

        return 1;
    }
    else if( raw.startsWith(act_u_set) )
    {
        // end of property contain ':'(1)
        int len = raw.length() - act_u_set.length() - 1;
        properties->action_u = raw.mid(act_u_set.length(), len);

        return 1;
    }

    else if( raw==act_d_clr )
    {
        properties->action_d = "";

        return 1;
    }
    else if( raw.startsWith(act_d_set) )
    {
        // end of property contain ':'(1)
        int len = raw.length() - act_d_set.length() - 1;
        properties->action_d = raw.mid(act_d_set.length(), len);

        return 1;
    }

    return 0;
}
