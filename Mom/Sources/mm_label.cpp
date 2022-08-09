#include "mm_label.h"

MmLabel::MmLabel()
{

}

void MmLabel::setVal(QString input)
{
    val  = "<div style = 'font-family: Roboto, ";
    val += "\"Font Awesome 6 Brands Regular\", ";
    val += "\"Font Awesome 6 Pro Solid\"'>";

    val += input.replace(" ", " &nbsp;");
    val += "</div>";
}
