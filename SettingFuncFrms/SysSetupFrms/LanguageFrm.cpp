#include "LanguageFrm.h"
#include "Config/ReadConfig.h"
#include "OperationTipsFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QFile>
#include <QApplication>
#include <QMouseEvent>
#include <QTranslator>
#include <QMessageBox>
#include <QStyle>

LanguageFrm* LanguageFrm::LangInst = nullptr;

// Custom Language Option Card Widget
class LanguageCardWidget : public QFrame
{
    Q_OBJECT
public:
    LanguageCardWidget(const QString &language, int value, QWidget *parent = nullptr)
        : QFrame(parent), m_value(value), m_selected(false)
    {
        setObjectName("LanguageCard");
        setupUI(language);
        setCursor(Qt::PointingHandCursor);
    }
    
    void setSelected(bool selected) {
        m_selected = selected;
        setProperty("selected", selected);
        style()->unpolish(this);
        style()->polish(this);
    }
    
    bool isSelected() const { return m_selected; }
    int getValue() const { return m_value; }

signals:
    void cardClicked(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        emit cardClicked(m_value);
    }

private:
    void setupUI(const QString &language) {
        setFixedHeight(60);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(20, 15, 20, 15);
        
        QLabel *languageLabel = new QLabel(language);
        languageLabel->setObjectName("LanguageText");
        
        QLabel *radioIndicator = new QLabel();
        radioIndicator->setObjectName("RadioIndicator");
        radioIndicator->setFixedSize(18, 18);
        
        layout->addWidget(languageLabel);
        layout->addStretch();
        layout->addWidget(radioIndicator);
    }

private:
    int m_value;
    bool m_selected;
};

class LanguageFrmPrivate
{
    Q_DECLARE_PUBLIC(LanguageFrm)
public:
    LanguageFrmPrivate(LanguageFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
    void SelectLanguage(int value);
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Language card
    LanguageCardWidget *m_pLanguageCard;
    int m_selectedValue;
    
    // Action buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
    
    QTranslator *m_trans;
private:
    LanguageFrm *const q_ptr;
};

LanguageFrmPrivate::LanguageFrmPrivate(LanguageFrm *dd)
    : q_ptr(dd), m_selectedValue(0)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
    m_trans = new QTranslator;
}

LanguageFrm::LanguageFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LanguageFrmPrivate(this))
{

}

LanguageFrm::~LanguageFrm()
{

}

LanguageFrm* LanguageFrm::GetLanguageInstance()
{
    if (nullptr == LangInst) {
        LangInst = new LanguageFrm;        
    }
    return LangInst;
}

void LanguageFrmPrivate::InitUI()
{
    q_func()->setObjectName("LanguageFrm");
    q_func()->setFixedSize(500, 350);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_pTitleLabel = new QLabel(QObject::tr("Language Settings"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(25, 25, 25, 25);
    contentLayout->setSpacing(25);
    
    // Language option (only English since other languages are commented out)
    QWidget *languageWidget = new QWidget;
    QVBoxLayout *languageLayout = new QVBoxLayout(languageWidget);
    languageLayout->setSpacing(15);
    
    m_pLanguageCard = new LanguageCardWidget(QObject::tr("English"), 0);
    languageLayout->addWidget(m_pLanguageCard);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 20, 0, 0);
    actionsLayout->setSpacing(15);
    actionsLayout->setAlignment(Qt::AlignCenter);
    
    m_pCancelButton = new QPushButton(QObject::tr("Cancel"));
    m_pCancelButton->setObjectName("CancelButton");
    
    m_pConfirmButton = new QPushButton(QObject::tr("OK"));
    m_pConfirmButton->setObjectName("ConfirmButton");
    
    actionsLayout->addWidget(m_pCancelButton);
    actionsLayout->addWidget(m_pConfirmButton);
    
    // Add separator line
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("DialogSeparator");
    
    contentLayout->addWidget(languageWidget);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void LanguageFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#LanguageFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
        "QWidget#DialogHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-top-left-radius: 15px;"
        "    border-top-right-radius: 15px;"
        "}"
        "QLabel#DialogTitle {"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        
        // Language card styles
        "LanguageCardWidget#LanguageCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 10px;"
        "}"
        "LanguageCardWidget#LanguageCard:hover {"
        "    border-color: #2196F3;"
        "    background: #f0f8ff;"
        "}"
        "LanguageCardWidget#LanguageCard[selected=\"true\"] {"
        "    border-color: #2196F3;"
        "    background: #e3f2fd;"
        "}"
        
        "QLabel#LanguageText {"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    color: #2c3e50;"
        "}"
        "LanguageCardWidget#LanguageCard[selected=\"true\"] QLabel#LanguageText {"
        "    color: #1976D2;"
        "}"
        
        "QLabel#RadioIndicator {"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 9px;"
        "    background: white;"
        "}"
        "LanguageCardWidget#LanguageCard[selected=\"true\"] QLabel#RadioIndicator {"
        "    border-color: #2196F3;"
        "    background: #2196F3;"
        "}"
        
        // Button styles
        "QPushButton#ConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#ConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#CancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#CancelButton:hover {"
        "    background: #545b62;"
        "}"
        
        // Separator
        "QFrame#DialogSeparator {"
        "    color: #e9ecef;"
        "    background-color: #e9ecef;"
        "}"
    );
}

void LanguageFrmPrivate::InitData()
{
    SelectLanguage(0); // Default to English
}

void LanguageFrmPrivate::InitConnect()
{
    // Connect language card selection
    QObject::connect(m_pLanguageCard, &LanguageCardWidget::cardClicked, 
                    [this](int value) {
                        SelectLanguage(value);
                    });
    
    // Connect buttons
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [this] {
        OperationTipsFrm dlg(q_func());
        if (0 != dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("ChangeLanguageHint"))) {
            return;
        }
        
        q_func()->setVisible(false);
        ReadConfig::GetInstance()->setLanguage_Mode(m_selectedValue);
        ReadConfig::GetInstance()->setSaveConfig();
        system("reboot");
    });
    
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [this] {
        q_func()->done(1);
    });
}

void LanguageFrmPrivate::SelectLanguage(int value)
{
    m_selectedValue = value;
    m_pLanguageCard->setSelected(true); // Since there's only one option
}

void LanguageFrm::setLanguageMode(const int &index)
{
    Q_D(LanguageFrm);
    d->SelectLanguage(index);
}

void LanguageFrm::UseLanguage(int forceload)
{
    Q_D(LanguageFrm);    
    
    QString strLanguageFile;
    int index = ReadConfig::GetInstance()->getLanguage_Mode();

    switch(index) {
        case 0:
        case 1:
        default:
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
    }
    
    if (QFile(strLanguageFile).exists()) {
        if (nullptr != d->m_trans)
            qApp->removeTranslator(d->m_trans);     
        d->m_trans->load(strLanguageFile);
        qApp->installTranslator(d->m_trans);
        emit LanguageChaned();
    }       
}

int LanguageFrm::getLanguageMode() const
{
    return d_func()->m_selectedValue;
}

void LanguageFrm::mousePressEvent(QMouseEvent *event)
{
    // Keep for backward compatibility
    int index = 0; // Only one language option
    this->setLanguageMode(index);           
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE
void LanguageFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "LanguageFrm.moc"