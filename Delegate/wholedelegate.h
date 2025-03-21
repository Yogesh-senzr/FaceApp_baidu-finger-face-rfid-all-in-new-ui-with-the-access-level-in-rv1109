#ifndef WHOLEDELEGATE_H
#define WHOLEDELEGATE_H

#include <QStyledItemDelegate>

class wholeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit wholeDelegate();
    ~wholeDelegate();
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // WHOLEDELEGATE_H
