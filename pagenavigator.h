#ifndef PAGENAVIGATOR_H
#define PAGENAVIGATOR_H

#include <QWidget>
#include <QList>
#include <QLabel>

namespace Ui
{
    class PageNavigator;
}

class PageNavigator : public QWidget
{
	Q_OBJECT
public:
	explicit PageNavigator(int blockSize = 3, QWidget *parent = nullptr);
	~PageNavigator();
public:
	inline int getBlockSize() const { return m_blockSize; }
	inline int getMaxPage() const{ return m_maxPage; }
	inline int getCurrentPage() const { return m_currentPage; }

    // 其他组件只需要调用这两个函数即可
    void setMaxPage(int page);   // 当总页数改变时调用
    void setCurrentPage(int page, bool signalEmitted = false); // 修改当前页时调用
private:
	void setBlockSize(int blockSize);
	void updatePageLabels();
	void initialize();
protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);
signals:
    void currentPageChanged(const int page);
private:
    Ui::PageNavigator *ui;
private:
    int m_blockSize;
    int m_maxPage;
    int m_currentPage;
    QList<QLabel *> *m_pageLabels;
};

#endif // PAGENAVIGATOR_H
