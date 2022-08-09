#include "mm_label.h"

MmLabel::MmLabel()
{

}

void MmLabel::setVal(QString input)
{
    val  = input.replace(" ", " &nbsp;");
}
