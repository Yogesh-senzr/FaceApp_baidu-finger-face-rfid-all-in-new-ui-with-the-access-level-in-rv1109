#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "RateModeDelegate.h"
#include <QComboBox>
#include <QStringList>

RateModeDelegate::RateModeDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *RateModeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	QComboBox *editor = new QComboBox(parent);
	editor->setObjectName("FComboBox");
	editor->addItems(QStringList() << "尖" << "峰" << "谷" << "平");
	return editor;
}

void RateModeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString text = index.model()->data(index, Qt::EditRole).toString();
	QComboBox *comboBox = static_cast<QComboBox*>(editor);
	comboBox->setCurrentIndex(comboBox->findText(text));
}

void RateModeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QComboBox *comboBox = static_cast<QComboBox*>(editor);
	QString text = comboBox->currentText();
	model->setData(index, text, Qt::EditRole);
}

void RateModeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
	editor->setGeometry(option.rect);
}

void RateModeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	// remove the focus state
	if (itemOption.state & QStyle::State_HasFocus)
	{
		itemOption.state ^= QStyle::State_HasFocus;
	}
	QStyledItemDelegate::paint(painter, itemOption, index);
}
