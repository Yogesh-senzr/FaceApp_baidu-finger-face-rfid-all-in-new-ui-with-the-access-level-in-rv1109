#ifndef PAGENAVIGATOR_H
#define PAGENAVIGATOR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class PageNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit PageNavigator(int blockSize = 7, QWidget *parent = nullptr);
    ~PageNavigator();

    int getCurrentPage() const { return m_currentPage; }
    int getMaxPage() const { return m_maxPage; }

public slots:
    void setMaxPage(int page);
    void setCurrentPage(int page, bool signalEmitted = true);
    void setBlockSize(int blockSize);

signals:
    void currentPageChanged(int page);

private slots:
    void onPageButtonClicked();
    void onPrevButtonClicked();
    void onNextButtonClicked();
    void onFirstButtonClicked();
    void onLastButtonClicked();

private:
    void setupUI();
    void applyModernStyles();
    void updatePageButtons();
    QPushButton* createNavButton(const QString &text, const QString &tooltip);
    QPushButton* createPageButton(int pageNumber);
    void animateButtonClick(QPushButton *button);

private:
    QHBoxLayout *m_mainLayout;
    QWidget *m_buttonContainer;
    QHBoxLayout *m_buttonLayout;
    
    QPushButton *m_firstButton;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QPushButton *m_lastButton;
    
    QList<QPushButton*> m_pageButtons;
    QPushButton *m_currentPageButton;
    
    int m_currentPage;
    int m_maxPage;
    int m_blockSize;
    
    QPropertyAnimation *m_clickAnimation;
};

#endif // PAGENAVIGATOR_H