#ifndef TABLEWIDGETDELEGATE_H
#define TABLEWIDGETDELEGATE_H

#include <QtWidgets/QWidget>
#include <QStyledItemDelegate>

class TableWidgetDelegate : public QStyledItemDelegate
{
public:
    TableWidgetDelegate(QWidget* parent = 0);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // TABLEWIDGETDELEGATE_H
