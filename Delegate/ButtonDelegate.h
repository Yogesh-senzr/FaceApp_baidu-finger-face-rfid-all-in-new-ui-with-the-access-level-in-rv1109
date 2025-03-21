#ifndef ButtonDelegate_H
#define ButtonDelegate_H

#include <QStyledItemDelegate>
class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ButtonDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    typedef QMap<QModelIndex, QPair<QStyleOptionButton*, QStyleOptionButton*>* >  collButtons;
    collButtons m_btns;
};

#endif // ButtonDelegate_H
