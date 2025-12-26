#include "SettingEditTextFrm.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFrame>
#include <QApplication>
#include <QDesktopWidget>

class SettingEditTextFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingEditTextFrm)
public:
    SettingEditTextFrmPrivate(SettingEditTextFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Content
    QFrame *m_pContentCard;
    QTextEdit *m_pTextEdit;
    
    // Action buttons  
    QPushButton *m_pLeftBtn;
    QPushButton *m_pRightBtn;
private:
    SettingEditTextFrm *const q_ptr;
};

SettingEditTextFrmPrivate::SettingEditTextFrmPrivate(SettingEditTextFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

SettingEditTextFrm::SettingEditTextFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowCloseButtonHint)
    , d_ptr(new SettingEditTextFrmPrivate(this))    
{
}

SettingEditTextFrm::~SettingEditTextFrm()
{
}

void SettingEditTextFrmPrivate::InitUI()
{
    q_func()->setObjectName("SettingEditTextFrm");
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("EditTextHeader");
    m_pHeaderWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_pTitleLabel = new QLabel(QObject::tr("Tips"));
    m_pTitleLabel->setObjectName("EditTextTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);
    
    // Content card
    m_pContentCard = new QFrame;
    m_pContentCard->setObjectName("EditTextContentCard");
    
    QVBoxLayout *cardLayout = new QVBoxLayout(m_pContentCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setObjectName("EditTextEditor");
    m_pTextEdit->setFont(QFont("SongTi", 8));
    m_pTextEdit->setContextMenuPolicy(Qt::NoContextMenu); 
    m_pTextEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pTextEdit->setReadOnly(true);
    
    cardLayout->addWidget(m_pTextEdit);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 20, 0, 0);
    actionsLayout->setSpacing(15);
    
    m_pLeftBtn = new QPushButton(QObject::tr("Ok"));
    m_pLeftBtn->setObjectName("EditTextOkButton");
    
    m_pRightBtn = new QPushButton(QObject::tr("Cancel"));
    m_pRightBtn->setObjectName("EditTextCancelButton");
    
    actionsLayout->addSpacing(10);
    actionsLayout->addWidget(m_pLeftBtn);
    actionsLayout->addWidget(m_pRightBtn);
    actionsLayout->addSpacing(10);
    
    // Add separator
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("DialogSeparator");
    
    contentLayout->addWidget(m_pContentCard, 20);
    contentLayout->addStretch();
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void SettingEditTextFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#SettingEditTextFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
        "QWidget#EditTextHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-top-left-radius: 15px;"
        "    border-top-right-radius: 15px;"
        "}"
        "QLabel#EditTextTitle {"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        
        // Content card styles
        "QFrame#EditTextContentCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 12px;"
        "}"
        
        // Text editor styles
        "QTextEdit#EditTextEditor {"
        "    background: white;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "    font-family: monospace;"
        "    font-size: 12px;"
        "}"
        
        // Button styles
        "QPushButton#EditTextOkButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "    height: 64px;"
        "}"
        "QPushButton#EditTextOkButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#EditTextCancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "    height: 64px;"
        "}"
        "QPushButton#EditTextCancelButton:hover {"
        "    background: #545b62;"
        "}"
        
        // Separator
        "QFrame#DialogSeparator {"
        "    color: #e9ecef;"
        "    background-color: #e9ecef;"
        "}"
    );
}

void SettingEditTextFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    if (width == 480)
        q_func()->setFixedSize(420, 760);
    else 
        q_func()->setFixedSize(640, 800-200+160);
    q_func()->move(80, 0+80);
}

void SettingEditTextFrmPrivate::InitConnect()
{
    QObject::connect(m_pLeftBtn, &QPushButton::clicked, [this] {
        q_func()->done(0);
    });
    QObject::connect(m_pRightBtn, &QPushButton::clicked, [this] {
        q_func()->done(1);
    });
}

void SettingEditTextFrm::setTitle(const QString &Name)
{
    Q_D(SettingEditTextFrm);
    d->m_pTitleLabel->setText(Name);
}

void SettingEditTextFrm::setData(const QString &Name)
{
    Q_D(SettingEditTextFrm);
    d->m_pTextEdit->setText(Name);
}

#ifdef SCREENCAPTURE
void SettingEditTextFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif