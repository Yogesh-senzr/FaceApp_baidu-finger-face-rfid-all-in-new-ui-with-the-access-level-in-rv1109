#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "Timedelegate.h"
#include <QTimeEdit>

TimeDelegate::TimeDelegate(QObject *parent): QStyledItemDelegate(parent)
{

}

QWidget *TimeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    QTimeEdit *editor = new QTimeEdit(QTime::currentTime(), parent);
   // editor->setObjectName("FTimeEdit");
    editor->setDisplayFormat("hh:mm");
    return editor;
}

//void TimeDelegate::setEditorData(QWidget *editor, const QModelIndex &/*index*/) const
//{//这里重写的好处就是可以把本地时间读出来写到表格里
//    //QTimeEdit *dateedit = static_cast<QTimeEdit*>(editor);
//   // dateedit->setDateTime;
//}

void TimeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QTimeEdit *dateedit = static_cast<QTimeEdit*>(editor);
    QString text = dateedit->text();
    model->setData(index, text, Qt::EditRole);
}

void TimeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

void TimeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    // remove the focus state
    if (itemOption.state & QStyle::State_HasFocus)
    {
        itemOption.state ^= QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, itemOption, index);
}
