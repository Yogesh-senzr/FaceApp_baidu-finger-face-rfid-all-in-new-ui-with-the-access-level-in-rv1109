#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "CheckBoxDelegate.h"
#include <QCheckBox>
#include <QStringList>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    QCheckBox *editor = new QCheckBox(parent);
    editor->setObjectName("FCheckBox");
    editor->setChecked(true);
	return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QCheckBox *comboBox = static_cast<QCheckBox*>(editor);
    comboBox->setChecked(text.startsWith(QObject::tr("放行")));
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QCheckBox *comboBox = static_cast<QCheckBox*>(editor);
    QString text = (comboBox->checkState() == Qt::Checked) ? QObject::tr("放行") : QObject::tr("禁行");
    model->setData(index, text, Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
	editor->setGeometry(option.rect);
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	// remove the focus state
	if (itemOption.state & QStyle::State_HasFocus)
	{
		itemOption.state ^= QStyle::State_HasFocus;
	}
	QStyledItemDelegate::paint(painter, itemOption, index);
}
