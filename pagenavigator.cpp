#include "pagenavigator.h"
#include <QTimer>
#include <QApplication>

PageNavigator::PageNavigator(int blockSize, QWidget *parent)
    : QWidget(parent)
    , m_currentPage(1)
    , m_maxPage(1)
    , m_blockSize(blockSize)
    , m_currentPageButton(nullptr)
    , m_clickAnimation(nullptr)
{
    setBlockSize(blockSize);
    setupUI();
    applyModernStyles();
    setMaxPage(1);
    setFixedHeight(60); // Increased from original to accommodate larger buttons
}

PageNavigator::~PageNavigator()
{
    if (m_clickAnimation) {
        m_clickAnimation->deleteLater();
    }
}
    
void PageNavigator::setupUI()
{
    setObjectName("PageNavigator");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // Main layout - REDUCED VERTICAL MARGINS
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 5, 20, 5); // Reduced vertical margins
    m_mainLayout->setSpacing(10);
    
    // Navigation buttons
    m_firstButton = createNavButton("«", tr("First Page"));
    m_prevButton = createNavButton("‹", tr("Previous Page"));
    m_nextButton = createNavButton("›", tr("Next Page"));
    m_lastButton = createNavButton("»", tr("Last Page"));
    
    // Button container
    m_buttonContainer = new QWidget;
    m_buttonContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_buttonLayout = new QHBoxLayout(m_buttonContainer);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(5);
    
    // Add navigation buttons
    m_mainLayout->addWidget(m_firstButton);
    m_mainLayout->addWidget(m_prevButton);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_buttonContainer, 1);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_nextButton);
    m_mainLayout->addWidget(m_lastButton);
    
    // Connect signals
    connect(m_firstButton, &QPushButton::clicked, this, &PageNavigator::onFirstButtonClicked);
    connect(m_prevButton, &QPushButton::clicked, this, &PageNavigator::onPrevButtonClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &PageNavigator::onNextButtonClicked);
    connect(m_lastButton, &QPushButton::clicked, this, &PageNavigator::onLastButtonClicked);
}

void PageNavigator::applyModernStyles()
{
    setStyleSheet(
        // Main container - EXPAND VERTICALLY
        "QWidget#PageNavigator {"
        "    background: transparent;"
        "}"
        
        // Navigation buttons - EXPAND VERTICALLY
        "QPushButton#NavButton {"
        "    background: #f8f9fa;"
        "    border: 1px solid #e1e5e9;"
        "    color: #6c757d;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 45px;"  // Increased width
        "    min-height: 45px;" // Increased height
        "    max-height: 45px;" // Fixed height
        "    border-radius: 8px;"
        "}"
        "QPushButton#NavButton:hover {"
        "    background: #e9ecef;"
        "    border-color: #2196F3;"
        "    color: #2196F3;"
        "}"
        "QPushButton#NavButton:disabled {"
        "    background: #f1f3f4;"
        "    border-color: #e8eaed;"
        "    color: #dadce0;"
        "}"
        
        // Page buttons - EXPAND VERTICALLY
        "QPushButton#PageButton {"
        "    background: transparent;"
        "    border: 1px solid #e1e5e9;"
        "    color: #6c757d;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    min-width: 45px;"  // Increased width
        "    min-height: 45px;" // Increased height
        "    max-height: 45px;" // Fixed height
        "    border-radius: 8px;"
        "}"
        "QPushButton#PageButton:hover {"
        "    background: #f8f9fa;"
        "    border-color: #2196F3;"
        "    color: #2196F3;"
        "}"
        
        // Active page button - EXPAND VERTICALLY
        "QPushButton#PageButtonActive {"
        "    background: #2196F3;"
        "    border: 1px solid #2196F3;"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: 700;"
        "    min-width: 45px;"  // Increased width
        "    min-height: 45px;" // Increased height
        "    max-height: 45px;" // Fixed height
        "    border-radius: 8px;"
        "}"
        
        // Ellipsis label - EXPAND VERTICALLY
        "QLabel#EllipsisLabel {"
        "    color: #9e9e9e;"
        "    font-weight: bold;"
        "    font-size: 16px;"
        "    padding: 0 5px;"
        "    min-height: 45px;" // Match button height
        "    max-height: 45px;" // Fixed height
        "}"
    );
}

QPushButton* PageNavigator::createNavButton(const QString &text, const QString &tooltip)
{
    QPushButton *button = new QPushButton(text);
    button->setObjectName("NavButton");
    button->setFixedSize(45, 45); // Increased size
    button->setToolTip(tooltip);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QPushButton* PageNavigator::createPageButton(int pageNumber)
{
    QPushButton *button = new QPushButton(QString::number(pageNumber));
    button->setProperty("pageNumber", pageNumber);
    button->setFixedSize(45, 45); // Increased size
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip(tr("Page %1").arg(pageNumber));
    
    if (pageNumber == m_currentPage) {
        button->setObjectName("PageButtonActive");
        m_currentPageButton = button;
    } else {
        button->setObjectName("PageButton");
    }
    
    connect(button, &QPushButton::clicked, this, &PageNavigator::onPageButtonClicked);
    return button;
}

void PageNavigator::setMaxPage(int page)
{
    page = qMax(page, 1);
    if (m_maxPage != page) {
        m_maxPage = page;
        if (m_currentPage > m_maxPage) {
            m_currentPage = m_maxPage;
        }
        updatePageButtons();
    }
}

void PageNavigator::setCurrentPage(int page, bool signalEmitted)
{
    page = qMax(page, 1);
    page = qMin(page, m_maxPage);

    if (page != m_currentPage) {
        m_currentPage = page;
        updatePageButtons();

        if (signalEmitted) {
            emit currentPageChanged(page);
        }
    }
}

void PageNavigator::setBlockSize(int blockSize)
{
    blockSize = qMax(blockSize, 5);
    if (blockSize % 2 == 0) {
        ++blockSize;
    }
    m_blockSize = blockSize;
}

void PageNavigator::updatePageButtons()
{
    // Clear existing page buttons
    for (QPushButton *button : m_pageButtons) {
        m_buttonLayout->removeWidget(button);
        button->deleteLater();
    }
    m_pageButtons.clear();
    m_currentPageButton = nullptr;
    
    // Clear all items from layout
    QLayoutItem *item;
    while ((item = m_buttonLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        } else {
            delete item;
        }
    }
    
    if (m_maxPage <= 1) {
        // Hide navigation if only one page
        setVisible(false);
        return;
    }
    
    setVisible(true);
    
    // Calculate page range to display
    int startPage = 1;
    int endPage = m_maxPage;
    
    if (m_maxPage > m_blockSize) {
        int halfBlock = m_blockSize / 2;
        startPage = qMax(1, m_currentPage - halfBlock);
        endPage = qMin(m_maxPage, startPage + m_blockSize - 1);
        
        if (endPage - startPage + 1 < m_blockSize) {
            startPage = qMax(1, endPage - m_blockSize + 1);
        }
    }
    
    // Add page buttons
    if (startPage > 1) {
        QPushButton *firstBtn = createPageButton(1);
        m_pageButtons.append(firstBtn);
        m_buttonLayout->addWidget(firstBtn);
        
        if (startPage > 2) {
QLabel *ellipsis = new QLabel("...");
ellipsis->setObjectName("EllipsisLabel");
ellipsis->setAlignment(Qt::AlignCenter);
ellipsis->setFixedSize(35, 45); // Increased height to match buttons
m_buttonLayout->addWidget(ellipsis);
        }
    }
    
    for (int i = startPage; i <= endPage; ++i) {
        QPushButton *pageBtn = createPageButton(i);
        m_pageButtons.append(pageBtn);
        m_buttonLayout->addWidget(pageBtn);
    }
    
    if (endPage < m_maxPage) {
        if (endPage < m_maxPage - 1) {
            QLabel *ellipsis = new QLabel("...");
            ellipsis->setObjectName("EllipsisLabel");
            ellipsis->setAlignment(Qt::AlignCenter);
            ellipsis->setFixedSize(30, 40);
            m_buttonLayout->addWidget(ellipsis);
        }
        
        QPushButton *lastBtn = createPageButton(m_maxPage);
        m_pageButtons.append(lastBtn);
        m_buttonLayout->addWidget(lastBtn);
    }
    
    // Update button states
    m_firstButton->setEnabled(m_currentPage > 1);
    m_prevButton->setEnabled(m_currentPage > 1);
    m_nextButton->setEnabled(m_currentPage < m_maxPage);
    m_lastButton->setEnabled(m_currentPage < m_maxPage);
}

void PageNavigator::animateButtonClick(QPushButton *button)
{
    if (!button) return;
    
    if (m_clickAnimation) {
        m_clickAnimation->stop();
        m_clickAnimation->deleteLater();
    }
    
    m_clickAnimation = new QPropertyAnimation(button, "geometry");
    m_clickAnimation->setDuration(120);
    
    QRect originalGeometry = button->geometry();
    QRect scaledGeometry = originalGeometry.adjusted(2, 2, -2, -2);
    
    m_clickAnimation->setStartValue(scaledGeometry);
    m_clickAnimation->setEndValue(originalGeometry);
    m_clickAnimation->setEasingCurve(QEasingCurve::OutBack);
    m_clickAnimation->start();
}

void PageNavigator::onPageButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    animateButtonClick(button);
    
    int pageNumber = button->property("pageNumber").toInt();
    if (pageNumber != m_currentPage) {
        setCurrentPage(pageNumber, true);
    }
}

void PageNavigator::onPrevButtonClicked()
{
    animateButtonClick(m_prevButton);
    if (m_currentPage > 1) {
        setCurrentPage(m_currentPage - 1, true);
    }
}

void PageNavigator::onNextButtonClicked()
{
    animateButtonClick(m_nextButton);
    if (m_currentPage < m_maxPage) {
        setCurrentPage(m_currentPage + 1, true);
    }
}

void PageNavigator::onFirstButtonClicked()
{
    animateButtonClick(m_firstButton);
    setCurrentPage(1, true);
}

void PageNavigator::onLastButtonClicked()
{
    animateButtonClick(m_lastButton);
    setCurrentPage(m_maxPage, true);
}