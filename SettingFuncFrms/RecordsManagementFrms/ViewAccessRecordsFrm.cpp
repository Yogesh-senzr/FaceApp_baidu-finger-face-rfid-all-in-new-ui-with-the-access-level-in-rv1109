#include "ViewAccessRecordsFrm.h"
#include "Delegate/wholedelegate.h"
#include "pagenavigator.h"
#include "FacePngFrm.h"

#include "Threads/RecordsExport.h"
#include "OperationTipsFrm.h"
#include "Application/FaceApp.h"
#include "HttpServer/PostPersonRecordThread.h"

#ifdef Q_OS_LINUX
#include "USB/UsbObserver.h"
#endif

#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlDriver>
#include <QTabBar>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

#define PAGE (20)
#define DateTime QDateTime::currentDateTime().toString("yyyy-MM-dd")

// Custom Card Widget Class (unchanged)
class AccessRecordCard : public QFrame
{
    Q_OBJECT

public:
    explicit AccessRecordCard(QWidget *parent = nullptr);
    void setRecordData(const QString &name, const QString &passTime, const QString &imagePath);
    
protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_faceImageLabel;
    QLabel *m_nameLabel;
    QLabel *m_timeLabel;
    
    void setupUI();
    void setupShadowEffect();
};

class ViewAccessRecordsFrmPrivate
{
    Q_DECLARE_PUBLIC(ViewAccessRecordsFrm)
public:
    ViewAccessRecordsFrmPrivate(ViewAccessRecordsFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void AddRecordCard(const QString &name, const QString &passTime, const QString &imagePath);
    void SelectData(const QString &sql);
private:
    void ClearCardData();
private:
    // Filter Controls
    QDateEdit *m_pStartTimerEdit;
    QDateEdit *m_pEndTimerEdit;
    QCalendarWidget *m_pStartCalendarWidget; 
    QCalendarWidget *m_pEndCalendarWidget; 
    QPushButton *m_pQueryButton;
    QPushButton *m_pExportButton;
    QPushButton *m_pPushButton;  // NEW
    QPushButton *m_pCleanButton;
    QProgressDialog *m_pProgressDialog;
    QProgressDialog *m_pManualPushProgressDialog;  // NEW
    bool m_bManualPushInProgress;  // NEW
    
    // Card Layout
    QScrollArea *m_pScrollArea;
    QWidget *m_pCardsContainer;
    QGridLayout *m_pCardsLayout;
    QList<AccessRecordCard*> m_cardsList;
    
    // Page Navigation
    PageNavigator *m_pPageNavigator;
    
private:
    ViewAccessRecordsFrm *const q_ptr;
};

// AccessRecordCard Implementation (unchanged)
AccessRecordCard::AccessRecordCard(QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_TranslucentBackground, false);  // ADD THIS
    setAttribute(Qt::WA_NoSystemBackground, false);      // ADD THIS
    
    setupUI();
    // setupShadowEffect();  // COMMENT THIS OUT TO TEST!
}

void AccessRecordCard::setupUI()
{
    setFixedSize(350, 100);
    setFrameStyle(QFrame::Box);
    setStyleSheet(
        "AccessRecordCard {"
        "    background-color: white;"
        "    border: 1px solid #e0e6ed;"
        "    border-radius: 12px;"
        "}"
        "AccessRecordCard:hover {"
        "    border-color: #3498db;"
        "    background-color: #f8f9fa;"
        "}"
    );
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    // Face image
    m_faceImageLabel = new QLabel;
    m_faceImageLabel->setFixedSize(60, 60);
    m_faceImageLabel->setScaledContents(true);
    m_faceImageLabel->setAutoFillBackground(true); // FIX: Enable background fill
    m_faceImageLabel->setStyleSheet(
        "QLabel {"
        "    border: 2px solid #ecf0f1;"
        "    border-radius: 8px;"
        "    background-color: #f8f9fa;"
        "}"
    );
    
    // Info container - FIX: Set white background
    QWidget *infoContainer = new QWidget;
    infoContainer->setAutoFillBackground(true);
    QPalette infoPalette = infoContainer->palette();
    infoPalette.setColor(QPalette::Window, Qt::white);
    infoContainer->setPalette(infoPalette);
    infoContainer->setStyleSheet("QWidget { background-color: white; }");
    
    QVBoxLayout *infoLayout = new QVBoxLayout(infoContainer);
    infoLayout->setContentsMargins(0, 5, 0, 5);
    infoLayout->setSpacing(8);
    
    // Name label - FIX: Set white background
    m_nameLabel = new QLabel;
    m_nameLabel->setAutoFillBackground(true);
    m_nameLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    background-color: white;"
        "}"
    );
    
    // Time label - FIX: Set white background
    m_timeLabel = new QLabel;
    m_timeLabel->setAutoFillBackground(true);
    m_timeLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 13px;"
        "    color: #7f8c8d;"
        "    font-weight: 500;"
        "    background-color: white;"
        "}"
    );
    
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_timeLabel);
    infoLayout->addStretch();
    
    layout->addWidget(m_faceImageLabel);
    layout->addWidget(infoContainer, 1);
}

void AccessRecordCard::setupShadowEffect()
{
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(8);
    shadowEffect->setColor(QColor(0, 0, 0, 30));
    shadowEffect->setOffset(0, 2);
    setGraphicsEffect(shadowEffect);
}

void AccessRecordCard::setRecordData(const QString &name, const QString &passTime, const QString &imagePath)
{
    m_nameLabel->setText(name);
    m_timeLabel->setText(passTime);
    
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        m_faceImageLabel->setPixmap(pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QPixmap placeholder(60, 60);
        placeholder.fill(QColor("#ecf0f1"));
        QPainter painter(&placeholder);
        painter.setPen(QColor("#7f8c8d"));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "Face");
        m_faceImageLabel->setPixmap(placeholder);
    }
}

void AccessRecordCard::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    setStyleSheet(styleSheet() + 
        "AccessRecordCard {"
        "    transform: translateY(-2px);"
        "}"
    );
    
    if (graphicsEffect()) {
        QGraphicsDropShadowEffect *shadow = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect());
        if (shadow) {
            shadow->setBlurRadius(12);
            shadow->setColor(QColor(0, 0, 0, 50));
            shadow->setOffset(0, 4);
        }
    }
}

void AccessRecordCard::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    setupShadowEffect();
}

void AccessRecordCard::paintEvent(QPaintEvent *event)
{
    // CRITICAL FIX: Explicitly paint white background before everything else
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw solid white rounded rectangle
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(0, 0, -1, -1), 12, 12);
    painter.fillPath(path, QBrush(Qt::white));
    
    // Draw border
    painter.setPen(QPen(QColor("#e0e6ed"), 1));
    painter.drawPath(path);
    
    // Now let QFrame draw its content (children widgets)
    QFrame::paintEvent(event);
}

// ViewAccessRecordsFrmPrivate Implementation
ViewAccessRecordsFrmPrivate::ViewAccessRecordsFrmPrivate(ViewAccessRecordsFrm *dd)
    : q_ptr(dd)
    , m_bManualPushInProgress(false)
    , m_pManualPushProgressDialog(nullptr)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ViewAccessRecordsFrm::ViewAccessRecordsFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new ViewAccessRecordsFrmPrivate(this))
{
      // FIX: Set white background for the main widget to prevent camera feed from showing through

    // Compatible with Qt 5.9.4

    setAutoFillBackground(true);

    QPalette mainPalette = palette();

    mainPalette.setColor(QPalette::Window, Qt::white);

    setPalette(mainPalette);

    

    // Stylesheet approach for additional coverage (Qt 5.9.4 compatible)

    setStyleSheet("ViewAccessRecordsFrm { background-color: white; }");
}

ViewAccessRecordsFrm::~ViewAccessRecordsFrm()
{
}

void ViewAccessRecordsFrmPrivate::InitUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Filter container
    QWidget *filterContainer = new QWidget;
    filterContainer->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    padding: 20px;"
        "}"
    );
    
    QGraphicsDropShadowEffect *filterShadow = new QGraphicsDropShadowEffect(filterContainer);
    filterShadow->setBlurRadius(8);
    filterShadow->setColor(QColor(0, 0, 0, 30));
    filterShadow->setOffset(0, 2);
    filterContainer->setGraphicsEffect(filterShadow);
    
    QVBoxLayout *filterLayout = new QVBoxLayout(filterContainer);
    filterLayout->setSpacing(15);
    filterLayout->setContentsMargins(25, 25, 25, 25);
    
    // First row: Date pickers
    QHBoxLayout *dateRow = new QHBoxLayout;
    dateRow->setSpacing(20);
    
    QLabel *fromLabel = new QLabel("From:");
    fromLabel->setStyleSheet("font-weight: 500; color: #2c3e50;");
    m_pStartTimerEdit = new QDateEdit;
    m_pStartTimerEdit->setFixedSize(160, 40);
    
    QLabel *toLabel = new QLabel("To:");
    toLabel->setStyleSheet("font-weight: 500; color: #2c3e50;");
    m_pEndTimerEdit = new QDateEdit;
    m_pEndTimerEdit->setFixedSize(160, 40);
    
    dateRow->addWidget(fromLabel);
    dateRow->addWidget(m_pStartTimerEdit);
    dateRow->addWidget(toLabel);
    dateRow->addWidget(m_pEndTimerEdit);
    dateRow->addStretch();
    
    // Second row: Action buttons (UPDATED WITH PUSH BUTTON)
    QHBoxLayout *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(15);
    
    m_pQueryButton = new QPushButton("🔍 Search");
    m_pExportButton = new QPushButton("📤 Export"); 
    m_pPushButton = new QPushButton("📡 Push");  // NEW
    m_pCleanButton = new QPushButton("🗑️ Clean");
    
    QString buttonStyle = 
        "QPushButton {"
        "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        "                                stop: 0 #3498db, stop: 1 #2980b9);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    padding: 12px 24px;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        "                                stop: 0 #3cb0fd, stop: 1 #3498db);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        "                                stop: 0 #2980b9, stop: 1 #21618c);"
        "}";
    
    QString exportButtonStyle = buttonStyle;
    exportButtonStyle.replace("#3498db", "#95a5a6");
    exportButtonStyle.replace("#2980b9", "#7f8c8d");
    exportButtonStyle.replace("#3cb0fd", "#a6b5b6");
    exportButtonStyle.replace("#21618c", "#6c7b7d");
    
    // NEW: Push button style (green)
    QString pushButtonStyle = buttonStyle;
    pushButtonStyle.replace("#3498db", "#27ae60");
    pushButtonStyle.replace("#2980b9", "#229954");
    pushButtonStyle.replace("#3cb0fd", "#2ecc71");
    pushButtonStyle.replace("#21618c", "#1e8449");
    
    QString cleanButtonStyle = buttonStyle;
    cleanButtonStyle.replace("#3498db", "#e74c3c");
    cleanButtonStyle.replace("#2980b9", "#c0392b");
    cleanButtonStyle.replace("#3cb0fd", "#ff6b5a");
    cleanButtonStyle.replace("#21618c", "#a93226");
    
    m_pQueryButton->setStyleSheet(buttonStyle);
    m_pExportButton->setStyleSheet(exportButtonStyle);
    m_pPushButton->setStyleSheet(pushButtonStyle);  // NEW
    m_pCleanButton->setStyleSheet(cleanButtonStyle);
    
    buttonRow->addWidget(m_pQueryButton);
    buttonRow->addWidget(m_pExportButton);
    buttonRow->addWidget(m_pPushButton);  // NEW
    buttonRow->addWidget(m_pCleanButton);
    buttonRow->addStretch();
    
    filterLayout->addLayout(dateRow);
    filterLayout->addLayout(buttonRow);
    
    // Cards scroll area
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameStyle(QFrame::NoFrame);
    m_pScrollArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: #f5f7fa;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: #f1f1f1;"
        "    width: 12px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #c1c1c1;"
        "    border-radius: 6px;"
        "    min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #a1a1a1;"
        "}"
    );

    m_pCardsContainer = new QWidget;
    m_pCardsContainer->setAutoFillBackground(true);
    QPalette containerPalette = m_pCardsContainer->palette();
    containerPalette.setColor(QPalette::Window, QColor(0xf5, 0xf7, 0xfa));
    m_pCardsContainer->setPalette(containerPalette);
    m_pCardsContainer->setStyleSheet(
        "QWidget {"
        "    background-color: #f5f7fa;"
        "}"
    );

    m_pCardsLayout = new QGridLayout(m_pCardsContainer);
    m_pCardsLayout->setSpacing(15);
    m_pCardsLayout->setContentsMargins(15, 15, 15, 15);
    m_pCardsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_pScrollArea->setWidget(m_pCardsContainer);
    
    // Page navigator
    m_pPageNavigator = new PageNavigator;
    
    // Add to main layout
    mainLayout->addWidget(filterContainer);
    mainLayout->addWidget(m_pScrollArea, 1);
    mainLayout->addWidget(m_pPageNavigator);
    
    // Calendar widgets
    m_pStartCalendarWidget = new QCalendarWidget;
    m_pEndCalendarWidget = new QCalendarWidget;
}

void ViewAccessRecordsFrmPrivate::InitData()
{
    // Setup date edits
    m_pStartTimerEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pEndTimerEdit->setContextMenuPolicy(Qt::NoContextMenu);
    
    m_pStartTimerEdit->setCalendarPopup(true);
    m_pStartTimerEdit->setDisplayFormat("yyyy/MM/dd");
    m_pStartTimerEdit->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1));
    
    m_pEndTimerEdit->setCalendarPopup(true);
    m_pEndTimerEdit->setDisplayFormat("yyyy/MM/dd");
    m_pEndTimerEdit->setDate(QDate::currentDate());
    
    QFont font("黑体", 8);
    m_pStartTimerEdit->setFont(font);
    m_pEndTimerEdit->setFont(font);
    
    QString dateEditStyle = 
        "QDateEdit {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
        "                                      stop:0 rgba(250, 250, 250, 255),"
        "                                      stop:0.5 rgba(240, 240, 240, 255),"
        "                                      stop:1 rgba(230, 230, 230, 255));"
        "    border: 2px solid rgb(200, 200, 200);"
        "    border-radius: 8px;"
        "    padding-left: 10px;"
        "}"
        "QDateEdit:focus {"
        "    border-color: #3498db;"
        "}"
        "QDateEdit::drop-down {"
        "    width: 35px;"
        "    border-image: url(:/Images/triangledown.png);"
        "}";
    
    m_pStartTimerEdit->setStyleSheet(dateEditStyle);
    m_pEndTimerEdit->setStyleSheet(dateEditStyle);
    
    // Setup calendars
    m_pStartTimerEdit->setCalendarWidget(m_pStartCalendarWidget);
    m_pEndTimerEdit->setCalendarWidget(m_pEndCalendarWidget);
    
    QString calendarStyle = 
        "QCalendarWidget QWidget#qt_calendar_navigationbar { height:40px; }"
        "QMenu { font-size:10px; width: 150px; left: 20px;"
        "        background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333); }"
        "QToolButton { icon-size: 48px, 48px; background-color: qlineargradient(x1:0, y1:0, x2:0,"
        "              y2:1, stop: 0 #cccccc, stop: 1 #333333);"
        "              height: 100px; width: 200px; }"
        "QAbstractItemView { selection-background-color: rgb(255, 174, 0); }"
        "QListView { background-color:white; }";
    
    m_pStartCalendarWidget->setLocale(QLocale(QLocale::English));
    m_pStartCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_pStartCalendarWidget->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    m_pStartCalendarWidget->setFixedSize(500, 400);
    m_pStartCalendarWidget->setFont(font);
    m_pStartCalendarWidget->setStyleSheet(calendarStyle);
    
    m_pEndCalendarWidget->setLocale(QLocale(QLocale::English));
    m_pEndCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_pEndCalendarWidget->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    m_pEndCalendarWidget->setFixedSize(500, 400);
    m_pEndCalendarWidget->setFont(font);
    m_pEndCalendarWidget->setStyleSheet(calendarStyle);
    
    QWidget *calendarNavBar1 = m_pStartCalendarWidget->findChild<QWidget *>("qt_calendar_navigationbar");
    if (calendarNavBar1) {
        calendarNavBar1->setFixedSize(500, 40);
    }
    
    QWidget *calendarNavBar2 = m_pEndCalendarWidget->findChild<QWidget *>("qt_calendar_navigationbar");
    if (calendarNavBar2) {
        calendarNavBar2->setFixedSize(500, 40);
    }
}

void ViewAccessRecordsFrmPrivate::InitConnect()
{
    QObject::connect(m_pQueryButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotQueryButton);
    QObject::connect(m_pPageNavigator, &PageNavigator::currentPageChanged, q_func(), &ViewAccessRecordsFrm::slotCurrentPageChanged);
    QObject::connect(m_pExportButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotExportButton);
    QObject::connect(m_pPushButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotPushButton);  // NEW
    QObject::connect(m_pCleanButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotCleanButton);
}

void ViewAccessRecordsFrmPrivate::SelectData(const QString &sql)
{
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    query.prepare(sql);
    query.exec();
    while (query.next())
    {
#ifdef Q_OS_WIN
        this->AddRecordCard(
            query.value("name").toString(), 
            query.value("time").toString(), 
            "C:\\Users\\63279\\face_crop_image\\2021-10-20\\202110201951474_9.jpg"
        );
#else
        this->AddRecordCard(
            query.value("name").toString(), 
            query.value("time").toString(), 
            query.value("img_path").toString()
        );
#endif
    }
}

void ViewAccessRecordsFrmPrivate::ClearCardData()
{
    qDeleteAll(m_cardsList);
    m_cardsList.clear();
    
    QLayoutItem* item;
    while ((item = m_pCardsLayout->takeAt(0)) != nullptr) {
        delete item;
    }
}

void ViewAccessRecordsFrmPrivate::AddRecordCard(const QString &name, const QString &passTime, const QString &imagePath)
{
    AccessRecordCard *card = new AccessRecordCard(m_pCardsContainer);
    card->setRecordData(name, passTime, imagePath);
    
    m_cardsList.append(card);
    
    int row = m_cardsList.count() / 2;
    int col = (m_cardsList.count() - 1) % 2;
    
    m_pCardsLayout->addWidget(card, row, col);
}

// Static helper functions
static inline QString PackLikeselect(const QString &begin, const QString &end)
{
    if(begin.isEmpty() && end.isEmpty())return QString("select * from identifyrecord");
    else if(begin.isEmpty() && !end.isEmpty())return QString("select * from identifyrecord where time<='%1'").arg(end);
    else if(!begin.isEmpty() && end.isEmpty())return QString("select * from identifyrecord where time>='%1'").arg(begin);
    return QString("select * from identifyrecord where time>='%1' and time<='%2'").arg(begin).arg(end);
}

static inline int queryRowCount(QSqlQuery &query)
{
    int initialPos = query.at();
    int pos = 0;
    if (query.last()) {
        pos = query.at() + 1;
    }
    else {
        pos = 0;
    }
    query.seek(initialPos);
    return pos;
}

static inline int SelectDataCount(const QString &sql)
{
    int count = 0;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));

    query.exec(sql);
    if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        count = query.size();
    }
    else
    {
        count = queryRowCount(query);
    }

    int page = 0;
    if (count <= PAGE)
    {
        page = 1;
    }
    else
    {
        page = count / PAGE;
        if ((count % PAGE) != 0)++page;
    }
    return page;
}

// Slot implementations
void ViewAccessRecordsFrm::setEnter()
{
    Q_D(ViewAccessRecordsFrm);
    d->m_pQueryButton->setFocus();
    this->slotQueryButton();

    static bool Init = false;
    if(!Init)
    {
        Init = true;
        QObject::connect(qXLApp->GetRecordsExport(), &RecordsExport::sigExportProgressShell, this, &ViewAccessRecordsFrm::slotExportProgressShell);
        QObject::connect(this, &ViewAccessRecordsFrm::sigExportPersons, qXLApp->GetRecordsExport(), &RecordsExport::slotExportPersons);
        QObject::connect(this, &ViewAccessRecordsFrm::sigManualPushRecords,
                FaceApp::GetPostPersonRecordThread(), &PostPersonRecordThread::slotManualPushRecords);
        QObject::connect(FaceApp::GetPostPersonRecordThread(), &PostPersonRecordThread::sigManualPushProgress,
                this, &ViewAccessRecordsFrm::slotManualPushProgress);
        QObject::connect(FaceApp::GetPostPersonRecordThread(), &PostPersonRecordThread::sigManualPushComplete,
                this, &ViewAccessRecordsFrm::slotManualPushComplete);
    }
}

void ViewAccessRecordsFrm::slotCurrentPageChanged(const int page)
{
    Q_D(ViewAccessRecordsFrm);
    Q_UNUSED(page);
    d->m_pQueryButton->setFocus();
    d->ClearCardData();
    QString text = ::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59");
    d->SelectData((text + QString(" order by rid desc LIMIT %1,%2").arg((page - 1) *PAGE).arg(PAGE)));
}

void ViewAccessRecordsFrm::slotQueryButton()
{
    Q_D(ViewAccessRecordsFrm);
    d->ClearCardData();
    QString text = ::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59");
    d->m_pPageNavigator->setMaxPage(SelectDataCount(text));
    d->SelectData((text + QString(" order by rid desc LIMIT 0,%1").arg(PAGE)));
}

// NEW: Manual Push Button Slot
void ViewAccessRecordsFrm::slotPushButton()
{
    Q_D(ViewAccessRecordsFrm);
    
    if (d->m_bManualPushInProgress)
    {
        QMessageBox::information(this, QObject::tr("Push"), 
                                QObject::tr("Push operation already in progress"));
        return;
    }
    
    // Get selected date range
    QString startDateStr = d->m_pStartTimerEdit->text();
    QString endDateStr = d->m_pEndTimerEdit->text();
    
    QDateTime startDateTime = QDateTime::fromString(startDateStr + " 00:00:00", "yyyy/MM/dd HH:mm:ss");
    QDateTime endDateTime = QDateTime::fromString(endDateStr + " 23:59:59", "yyyy/MM/dd HH:mm:ss");
    
    // Validate date range
    if (!startDateTime.isValid() || !endDateTime.isValid())
    {
        QMessageBox::warning(this, QObject::tr("Push"), 
                           QObject::tr("Invalid date range selected"));
        return;
    }
    
    if (startDateTime > endDateTime)
    {
        QMessageBox::warning(this, QObject::tr("Push"), 
                           QObject::tr("Start date cannot be after end date"));
        return;
    }
    
    // Confirm with user
    QString confirmMsg = QString("Push all unsent attendance records from:\n%1\nto:\n%2\n\nContinue?")
                        .arg(startDateTime.toString("yyyy/MM/dd HH:mm:ss"))
                        .arg(endDateTime.toString("yyyy/MM/dd HH:mm:ss"));
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, QObject::tr("Confirm Push"), confirmMsg,
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes)
    {
        return;
    }
    
    // Disable push button and show progress dialog
    d->m_pPushButton->setEnabled(false);
    d->m_pPushButton->setText(QObject::tr("Pushing..."));
    d->m_bManualPushInProgress = true;
    
    // Create progress dialog
    d->m_pManualPushProgressDialog = new QProgressDialog(this);
    d->m_pManualPushProgressDialog->setWindowModality(Qt::WindowModal);
    d->m_pManualPushProgressDialog->setAttribute(Qt::WA_DeleteOnClose, false);
    d->m_pManualPushProgressDialog->setMinimumDuration(0);
    d->m_pManualPushProgressDialog->setWindowTitle(QObject::tr("Pushing Records"));
    d->m_pManualPushProgressDialog->setLabelText(QObject::tr("Pushing attendance records to server..."));
    d->m_pManualPushProgressDialog->setCancelButton(nullptr);
    d->m_pManualPushProgressDialog->setRange(0, 100);
    d->m_pManualPushProgressDialog->setValue(0);
    d->m_pManualPushProgressDialog->show();
    
    // Emit signal to start manual push in background thread
    emit sigManualPushRecords(startDateTime, endDateTime);
}

// NEW: Handle push progress updates
void ViewAccessRecordsFrm::slotManualPushProgress(int current, int total, bool success)
{
    Q_D(ViewAccessRecordsFrm);
    
    if (!d->m_pManualPushProgressDialog)
        return;
    
    // Update progress
    d->m_pManualPushProgressDialog->setMaximum(total);
    d->m_pManualPushProgressDialog->setValue(current);
    
    // Update label with current status
    QString statusMsg = QString("Pushed %1 of %2 records...").arg(current).arg(total);
    d->m_pManualPushProgressDialog->setLabelText(statusMsg);
    
    // Process events to keep UI responsive
    QCoreApplication::processEvents();
}

// NEW: Handle push completion
void ViewAccessRecordsFrm::slotManualPushComplete(bool success, QString message)
{
    Q_D(ViewAccessRecordsFrm);
    
    // Close progress dialog
    if (d->m_pManualPushProgressDialog)
    {
        d->m_pManualPushProgressDialog->close();
        delete d->m_pManualPushProgressDialog;
        d->m_pManualPushProgressDialog = nullptr;
    }
    
    // Re-enable push button
    d->m_pPushButton->setEnabled(true);
    d->m_pPushButton->setText(QObject::tr("📡 Push"));
    d->m_bManualPushInProgress = false;
    
    // Show result message
    if (success)
    {
        QMessageBox::information(this, QObject::tr("Push Complete"), message);
        
        // Refresh the display to show updated records
        slotQueryButton();
    }
    else
    {
        QMessageBox::warning(this, QObject::tr("Push Failed"), message);
    }
}

void ViewAccessRecordsFrm::slotExportButton()
{
    Q_D(ViewAccessRecordsFrm);
#ifdef Q_OS_LINUX
    
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState)
    {
        d->m_pExportButton->setEnabled(false);
        d->m_pExportButton->setText(QObject::tr("Export..."));
        d->m_pProgressDialog = new QProgressDialog(this);
        d->m_pProgressDialog->setWindowModality(Qt::WindowModal);
        d->m_pProgressDialog->setAttribute(Qt::WA_DeleteOnClose,true);
        d->m_pProgressDialog->setMinimumDuration(5);
        d->m_pProgressDialog->setWindowTitle(QObject::tr("PlsWaiting"));
        d->m_pProgressDialog->setLabelText(QObject::tr("Export..."));
        d->m_pProgressDialog->setCancelButtonText(NULL);        
        emit sigExportPersons(::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59"));
    }else
    {
        OperationTipsFrm dlg(this);
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("PlsInsertUDisk"), QObject::tr("Ok"), QString(), 1);
    }
#else
    d->m_pExportButton->setEnabled(false);
    d->m_pExportButton->setText(QObject::tr("Export..."));
    emit sigExportPersons(::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59"));
#endif
}

void ViewAccessRecordsFrm::slotCleanButton()
{
    Q_D(ViewAccessRecordsFrm);
    char cmdline[256];
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, QObject::tr("Tips"), QObject::tr("PurgeHint"),
        QMessageBox::Yes|QMessageBox::No);
    if (btn ==QMessageBox::Yes)   {

        sprintf(cmdline,"%s","sqlite3 /mnt/user/facedb/isc_ir_arcsoft_face.db 'delete from identifyrecord ';");      
        qDebug()<<"清除记录 cmdline: "<<cmdline;
        system(cmdline);

        sprintf(cmdline,"find /mnt/user/face_crop_image/ -name *.jpg  | awk  ' {print $0 } ' | xargs rm -rf ");        
        qDebug()<<"清除图片 cmdline: "<<cmdline;
    
        system(cmdline);

        d->ClearCardData();
    }
}

void ViewAccessRecordsFrm::slotExportProgressShell(const bool dealstate, const bool savestate, const int total, const int dealcnt)
{
    Q_D(ViewAccessRecordsFrm);
    Q_UNUSED(savestate);
    Q_UNUSED(total);
    Q_UNUSED(dealcnt);
    if(dealstate)
    {
        d->m_pProgressDialog->setRange(0, total);    
        d->m_pProgressDialog->setValue(dealcnt);
        d->m_pProgressDialog->setModal(true);             
        d->m_pProgressDialog->show();
        
        if (total == dealcnt)
        {
            
            QTime dieTime = QTime::currentTime().addMSecs(500);

            while( QTime::currentTime() < dieTime )
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        
            d->m_pExportButton->setEnabled(true);
            d->m_pExportButton->setText(QObject::tr("📤 Export"));
            
            d->m_pProgressDialog->close();
            d->m_pProgressDialog = NULL;
        
        }
    }
}

void ViewAccessRecordsFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}

#include "ViewAccessRecordsFrm.moc"