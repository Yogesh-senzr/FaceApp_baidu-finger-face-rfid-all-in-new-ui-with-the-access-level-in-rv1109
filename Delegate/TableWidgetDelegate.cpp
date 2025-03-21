#include "TableWidgetDelegate.h"
#include <QTableWidget>

TableWidgetDelegate::TableWidgetDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{

}

void TableWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QTableWidget* pTable =  static_cast<QTableWidget*> (this->parent());
    if (pTable != nullptr) {
        if (index.column() == 0) {  //列标为0
            //int tableWidth = pTable->width();   //获取表格宽度
            int width = pTable->width() * 1 / 5;    //按比例设置列宽
            pTable->setColumnWidth(0, width);   //设置列宽
            pTable->setRowHeight(index.row(), 20);  //设置行高

        }
        if (index.column() == 1) {
            //int tableWidth = pTable->width();
            int width = pTable->width() * 2 / 5;
            pTable->setColumnWidth(1, width);
            QString strValue = index.model()->data(index, Qt::DisplayRole).toString();
            QFont font;
            font.setFamily("Microsoft YaHei");
            font.setPointSize(8);
            QFontMetrics fm(font);
            QRect rec = fm.boundingRect(strValue);
            //字符串所占的像素宽度,高度
            int textWidth = rec.width();
            int textHeight = rec.height();
            if (textWidth > width) {
                if (textWidth / width == 0) {
                    pTable->setRowHeight(index.row(), textHeight);
                } else {
                    int hh = textWidth / width + 2;
                    int height = 20 * hh;
                    int row = index.row();
                    pTable->setRowHeight(row, height);
                }

            }
        }
        if (index.column() == 2) {
            //int tableWidth = pTable->width();
            int width = pTable->width() * 1 / 5;
            pTable->setColumnWidth(2, width);
        }
        if (index.column() == 3) {
            //int tableWidth = pTable->width();
            int width = pTable->width() * 1 / 5;
            pTable->setColumnWidth(3, width);
        }


    }
    QStyledItemDelegate::paint(painter, option, index);
}
