// Modified RecordsManagementFrm.cpp - Card-based theme implementation

#include "RecordsManagementFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "ConfigAccessRecordsFrm.h"
#include "Config/ReadConfig.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QtConcurrent/QtConcurrent>

// Custom Records Management Card Widget
class RecordsManagementCardWidget : public QFrame
{
    Q_OBJECT
public:
    RecordsManagementCardWidget(const QString &title, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title)
    {
        setObjectName("RecordsCardWidget");
        setupUI(title);
    }
    
    QString getTitle() const { return m_title; }
    
    void setDescription(const QString &description)
    {
        if (m_descLabel) {
            m_descLabel->setText(description);
        }
    }

signals:
    void cardClicked(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit cardClicked(m_title);
        QFrame::mousePressEvent(event);
    }
    
    void enterEvent(QEvent *event) override
    {
        setProperty("hovered", true);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override
    {
        setProperty("hovered", false);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::leaveEvent(event);
    }

private:
    void setupUI(const QString &title)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(40, 40);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setText(getIconText(title));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("RecordsIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("RecordsTitleLabel");
        
        m_descLabel = new QLabel(getDescription(title));
        m_descLabel->setObjectName("RecordsDescLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(m_descLabel);
        
        // Arrow
        QLabel *arrowLabel = new QLabel("›");
        arrowLabel->setObjectName("RecordsArrowLabel");
        
        // Main layout assembly
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(arrowLabel);
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Setting") || title.contains("Config"))
            return "SettingRecordsIconFrame";
        else if (title.contains("View") || title.contains("Display"))
            return "ViewRecordsIconFrame";
        return "DefaultRecordsIconFrame";
    }
    
    QString getIconText(const QString &title)
    {
        if (title.contains("Setting") || title.contains("Config"))
            return "";
        else if (title.contains("View") || title.contains("Display"))
            return "";
        return "";
    }
    
    QString getDescription(const QString &title)
    {
        if (title.contains("Setting") || title.contains("Config"))
            return "Configure access record settings";
        else if (title.contains("View") || title.contains("Display"))
            return "View historical access records";
        return "Records management";
    }

private:
    QString m_title;
    QLabel *m_descLabel;
};

class RecordsManagementFrmPrivate
{
    Q_DECLARE_PUBLIC(RecordsManagementFrm)
public:
    RecordsManagementFrmPrivate(RecordsManagementFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void ConfigAccessRecords();
private:
    QVBoxLayout *m_pMainLayout;
  //  QLabel *m_pHeaderLabel;
   // QFrame *m_pHeaderFrame;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<RecordsManagementCardWidget*> m_cardWidgets;
private:
    RecordsManagementFrm *const q_ptr;
};

RecordsManagementFrmPrivate::RecordsManagementFrmPrivate(RecordsManagementFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void RecordsManagementFrmPrivate::InitUI()
{
    Q_Q(RecordsManagementFrm);
    
    // Set main widget object name for styling
    q->setObjectName("RecordsManagementMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(20);
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("RecordsManagementScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("RecordsManagementContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(8);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout
   // m_pMainLayout->addWidget(m_pHeaderFrame);
    m_pMainLayout->addWidget(m_pScrollArea);
}

void RecordsManagementFrmPrivate::InitData()
{
    LanguageFrm::GetInstance()->UseLanguage(0);
    
    QStringList listName;
    listName << QObject::tr("SettingPassRecord") << QObject::tr("ViewPassRecord");
    
    for(int i = 0; i < listName.count(); i++)
    {
        RecordsManagementCardWidget *cardWidget = new RecordsManagementCardWidget(listName.at(i));
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    // Add stretch at the end
    m_pContentLayout->addStretch();
}

void RecordsManagementFrmPrivate::InitConnect()
{
    for(RecordsManagementCardWidget *card : m_cardWidgets)
    {
        QObject::connect(card, &RecordsManagementCardWidget::cardClicked, 
                        [this](const QString &title) {
            Q_Q(RecordsManagementFrm);
            q->handleCardClicked(title);
        });
    }
}

void RecordsManagementFrmPrivate::ConfigAccessRecords()
{
    ConfigAccessRecordsFrm dlg(q_func());
    dlg.setInitConfig(ReadConfig::GetInstance()->getRecords_Manager_PanoramaImg(), 
                      ReadConfig::GetInstance()->getRecords_Manager_FaceImg(), 
                      ReadConfig::GetInstance()->getRecords_Manager_Stranger());
    if(dlg.exec()) return;
    
    ReadConfig::GetInstance()->setRecords_Manager_PanoramaImg(dlg.getPanoramaState());
    ReadConfig::GetInstance()->setRecords_Manager_FaceImg(dlg.getFaceState());
    ReadConfig::GetInstance()->setRecords_Manager_Stranger(dlg.getStrangerState());
}

// Rest of your existing RecordsManagementFrm implementation...
RecordsManagementFrm::RecordsManagementFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new RecordsManagementFrmPrivate(this))
{
}

RecordsManagementFrm::~RecordsManagementFrm()
{
}

void RecordsManagementFrm::setLeaveEvent()
{
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void RecordsManagementFrm::handleCardClicked(const QString &title)
{
    Q_D(RecordsManagementFrm);
    
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
#endif
    
    if(title.startsWith(QObject::tr("ViewPassRecord"))) {
        emit sigShowFrm(title);
    } else {
        d->ConfigAccessRecords();
    }
}

void RecordsManagementFrm::slotIemClicked(QListWidgetItem *item)
{
    // Keep for compatibility - functionality moved to handleCardClicked
    // This method is no longer used in the card-based implementation
    // but kept for backward compatibility
    Q_UNUSED(item);
}

#ifdef SCREENCAPTURE
void RecordsManagementFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "RecordsManagementFrm.moc" // For RecordsManagementCardWidget Q_OBJECT