#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif

#include "Numberdelegate.h"
#include <QRegExp>
#include <QLineEdit>

NumberDelegate::NumberDelegate(QObject *parent): QStyledItemDelegate(parent)
{

}

QWidget *NumberDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
  QLineEdit *editor = new QLineEdit(parent);
  QRegExp regExp("^[-+]?[0-9]+(.[0-9]+)?$");
  editor->setValidator(new QRegExpValidator(regExp, parent));
  return editor;
}

void NumberDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
  QString text = lineEdit->text();
  if (text.isEmpty())text = "0.00";
  model->setData(index, text, Qt::EditRole);
}

void NumberDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
  editor->setGeometry(option.rect);
}

void NumberDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItem itemOption(option);
  // remove the focus state
  if (itemOption.state & QStyle::State_HasFocus)
    {
      itemOption.state ^= QStyle::State_HasFocus;
    }
  QStyledItemDelegate::paint(painter, itemOption, index);
}
