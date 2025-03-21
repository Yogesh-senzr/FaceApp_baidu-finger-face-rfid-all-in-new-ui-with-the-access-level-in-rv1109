#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "wholedelegate.h"

wholeDelegate::wholeDelegate()
{

}

wholeDelegate::~wholeDelegate()
{

}

void wholeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    // remove the focus state
    if (itemOption.state & QStyle::State_HasFocus)
    {
        itemOption.state ^= QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, itemOption, index);
}
