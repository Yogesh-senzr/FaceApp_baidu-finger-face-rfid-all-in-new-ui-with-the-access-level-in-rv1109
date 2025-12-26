#include "ConfigAccessRecordsFrm.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QStyle>
#include <QVariant>

// Custom Toggle Switch Widget
class ToggleSwitch : public QFrame
{
    Q_OBJECT
public:
    explicit ToggleSwitch(QWidget *parent = nullptr) : QFrame(parent), m_checked(false)
    {
        setObjectName("ToggleSwitch");
        setFixedSize(60, 30);
        setCursor(Qt::PointingHandCursor);
        
        m_slider = new QFrame(this);
        m_slider->setObjectName("ToggleSlider");
        m_slider->setFixedSize(24, 24);
        m_slider->move(3, 3);
        
        updateSliderPosition();
    }
    
    bool isChecked() const { return m_checked; }
    
    void setChecked(bool checked) {
        if (m_checked != checked) {
            m_checked = checked;
            updateSliderPosition();
            updateStyles();
            emit toggled(m_checked);
        }
    }

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        setChecked(!m_checked);
    }
    
    void enterEvent(QEvent *event) override {
        Q_UNUSED(event);
        setProperty("hovered", QVariant(true));
        style()->unpolish(this);
        style()->polish(this);
    }
    
    void leaveEvent(QEvent *event) override {
        Q_UNUSED(event);
        setProperty("hovered", QVariant(false));
        style()->unpolish(this);
        style()->polish(this);
    }

private:
    void updateSliderPosition() {
        int x = m_checked ? 33 : 3; // 60 - 24 - 3 = 33
        m_slider->move(x, 3);
    }
    
    void updateStyles() {
        setProperty("checked", QVariant(m_checked));
        style()->unpolish(this);
        style()->polish(this);
    }
    
private:
    QFrame *m_slider;
    bool m_checked;
};

// Custom Configuration Card Widget
class ConfigCard : public QFrame
{
    Q_OBJECT
public:
    explicit ConfigCard(const QString &title, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("ConfigCard");
        setupUI(title);
    }
    
    bool isChecked() const { return m_toggle->isChecked(); }
    void setChecked(bool checked) { m_toggle->setChecked(checked); }
    
    ToggleSwitch* getToggle() const { return m_toggle; }

private:
    void setupUI(const QString &title) {
        setFixedHeight(80);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(25, 20, 25, 20);
        layout->setSpacing(20);
        
        // Title label
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("ConfigTitle");
        
        // Toggle switch
        m_toggle = new ToggleSwitch;
        
        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(m_toggle);
    }
    
private:
    ToggleSwitch *m_toggle;
};

class ConfigAccessRecordsFrmPrivate
{
    Q_DECLARE_PUBLIC(ConfigAccessRecordsFrm)
public:
    ConfigAccessRecordsFrmPrivate(ConfigAccessRecordsFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyModernStyles();
    void CreateHeader();
    void CreateConfigSection();
    void CreateButtonSection();
private:
    // UI Components
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pMainLayout;
    
    // Configuration cards
    ConfigCard *m_pSaveFullShotCard;
    ConfigCard *m_pSaveFaceCard;
    ConfigCard *m_pSaveStrangerCard;
    
    // Buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
    
private:
    ConfigAccessRecordsFrm *const q_ptr;
};

ConfigAccessRecordsFrmPrivate::ConfigAccessRecordsFrmPrivate(ConfigAccessRecordsFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyModernStyles();
}

ConfigAccessRecordsFrm::ConfigAccessRecordsFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new ConfigAccessRecordsFrmPrivate(this))
{
    setFixedSize(500, 650);
    
    // Center the dialog
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
}

ConfigAccessRecordsFrm::~ConfigAccessRecordsFrm()
{

}

void ConfigAccessRecordsFrmPrivate::InitUI()
{
    LanguageFrm::GetInstance()->UseLanguage(0);
    
    q_func()->setObjectName("ConfigAccessRecordsFrm");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q_func());
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainLayout->setSpacing(0);
    
    CreateHeader();
    
    // Content widget (no scroll area)
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("ContentWidget");
    
    QVBoxLayout *contentLayout = new QVBoxLayout(m_pContentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);
    
    CreateConfigSection();
    CreateButtonSection();
    
    contentLayout->addStretch();
    
    m_pMainLayout->addWidget(m_pContentWidget, 1);
}

void ConfigAccessRecordsFrmPrivate::CreateHeader()
{
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(80);
    
    QVBoxLayout *headerLayout = new QVBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(30, 25, 30, 25);
    
    m_pTitleLabel = new QLabel;
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    m_pMainLayout->addWidget(m_pHeaderWidget);
}

void ConfigAccessRecordsFrmPrivate::CreateConfigSection()
{
    QVBoxLayout *configLayout = qobject_cast<QVBoxLayout*>(m_pContentWidget->layout());
    
    // Create configuration cards
    m_pSaveFullShotCard = new ConfigCard(QObject::tr("SaveFullPicture"));
    m_pSaveFaceCard = new ConfigCard(QObject::tr("SaveFacePicture"));
    m_pSaveStrangerCard = new ConfigCard(QObject::tr("SaveNewPersonRecord"));
    
    configLayout->addWidget(m_pSaveFullShotCard);
    configLayout->addWidget(m_pSaveFaceCard);
    configLayout->addWidget(m_pSaveStrangerCard);
}

void ConfigAccessRecordsFrmPrivate::CreateButtonSection()
{
    QWidget *buttonWidget = new QWidget;
    buttonWidget->setObjectName("ButtonWidget");
    
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 20, 0, 0);
    buttonLayout->setSpacing(15);
    
    m_pConfirmButton = new QPushButton;
    m_pConfirmButton->setObjectName("ConfirmButton");
    m_pConfirmButton->setFixedHeight(50);
    
    m_pCancelButton = new QPushButton;
    m_pCancelButton->setObjectName("CancelButton");
    m_pCancelButton->setFixedHeight(50);
    
    buttonLayout->addWidget(m_pConfirmButton);
    buttonLayout->addWidget(m_pCancelButton);
    
    qobject_cast<QVBoxLayout*>(m_pContentWidget->layout())->addWidget(buttonWidget);
}

void ConfigAccessRecordsFrmPrivate::InitData()
{
    m_pTitleLabel->setText(QObject::tr("settingPassRecord"));
    m_pConfirmButton->setText(QObject::tr("Ok"));
    m_pCancelButton->setText(QObject::tr("Cancel"));
    
    // Set initial states
    m_pSaveFullShotCard->setChecked(true);
    m_pSaveFaceCard->setChecked(true);
    m_pSaveStrangerCard->setChecked(false);
}

void ConfigAccessRecordsFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] { q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] { q_func()->done(1); });
}

void ConfigAccessRecordsFrmPrivate::ApplyModernStyles()
{
    q_func()->setStyleSheet(
        // Main dialog
        "QDialog#ConfigAccessRecordsFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header
        "QWidget#DialogHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-radius: 15px 15px 0px 0px;"
        "}"
        "QLabel#DialogTitle {"
        "    color: white;"
        "    font-size: 24px;"
        "    font-weight: 600;"
        "}"
        
        // Content area
        "QScrollArea#ContentScrollArea {"
        "    border: none;"
        "    background: white;"
        "}"
        "QWidget#ContentWidget {"
        "    background: white;"
        "}"
        
        // Configuration cards
        "QFrame#ConfigCard {"
        "    background: #f8f9fa;"
        "    border-radius: 12px;"
        "    border-left: 4px solid #2196F3;"
        "    min-height: 80px;"
        "}"
        "QFrame#ConfigCard:hover {"
        "    border-left-color: #1976D2;"
        "    box-shadow: 0 2px 10px rgba(0,0,0,0.1);"
        "}"
        "QLabel#ConfigTitle {"
        "    font-size: 18px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "}"
        
        // Toggle switch
        "QFrame#ToggleSwitch {"
        "    background: #e9ecef;"
        "    border-radius: 15px;"
        "    border: 1px solid #dee2e6;"
        "}"
        "QFrame#ToggleSwitch[checked=\"true\"] {"
        "    background: #4CAF50;"
        "    border: 1px solid #4CAF50;"
        "}"
        "QFrame#ToggleSlider {"
        "    background: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #dee2e6;"
        "}"
        
        // Buttons
        "QPushButton#ConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#ConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#CancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#CancelButton:hover {"
        "    background: #545b62;"
        "}"
    );
}

// Public interface methods
void ConfigAccessRecordsFrm::setInitConfig(const bool state, const bool state1, const bool state2)
{
    Q_D(ConfigAccessRecordsFrm);
    d->m_pSaveFullShotCard->setChecked(state);
    d->m_pSaveFaceCard->setChecked(state1);
    d->m_pSaveStrangerCard->setChecked(state2);
}

bool ConfigAccessRecordsFrm::getPanoramaState() const
{
    return d_func()->m_pSaveFullShotCard->isChecked();
}

bool ConfigAccessRecordsFrm::getFaceState() const
{
    return d_func()->m_pSaveFaceCard->isChecked();
}

bool ConfigAccessRecordsFrm::getStrangerState() const
{
    return d_func()->m_pSaveStrangerCard->isChecked();
}

#ifdef SCREENCAPTURE
void ConfigAccessRecordsFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "ConfigAccessRecordsFrm.moc"