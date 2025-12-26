// Fixed ManagingPeopleFrm.cpp - No header with PNG icon support

#include "ManagingPeopleFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QPixmap>

// Custom People Management Card Widget
class PeopleManagementCardWidget : public QFrame
{
    Q_OBJECT
public:
    PeopleManagementCardWidget(const QString &title, const QString &iconPath, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_iconPath(iconPath)
    {
        setObjectName("PeopleCardWidget");
        setupUI(title, iconPath);
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
    void setupUI(const QString &title, const QString &iconPath)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(48, 48);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("PeopleIconLabel");
        
        // Load PNG icon
        QPixmap iconPixmap(iconPath);
        if (!iconPixmap.isNull()) {
            // Scale the icon to fit within the frame (32x32 pixels)
            QPixmap scaledPixmap = iconPixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            iconLabel->setPixmap(scaledPixmap);
        } else {
            // Fallback to simple text if PNG not found
            iconLabel->setText(getFallbackIconText(title));
        }
        
        iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("PeopleTitleLabel");
        
        m_descLabel = new QLabel(getDescription(title));
        m_descLabel->setObjectName("PeopleDescLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(m_descLabel);
        
        // Arrow
        QLabel *arrowLabel = new QLabel(">");
        arrowLabel->setObjectName("PeopleArrowLabel");
        
        // Main layout assembly
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(arrowLabel);
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Add") || title.contains("AddPerson"))
            return "AddPersonIconFrame";
        else if (title.contains("Import"))
            return "ImportPersonIconFrame";
        else if (title.contains("Inquery") || title.contains("Query") || title.contains("Search"))
            return "QueryPersonIconFrame";
        else if (title.contains("Fingerprint") || title.contains("AddFingerprint"))
            return "AddFingerprintIconFrame";
        return "DefaultPeopleIconFrame";
    }
    
    QString getFallbackIconText(const QString &title)
    {
        if (title.contains("Add") || title.contains("AddPerson"))
            return "+";
        else if (title.contains("Import"))
            return "↑";
        else if (title.contains("Inquery") || title.contains("Query") || title.contains("Search"))
            return "🔍";
        else if (title.contains("Fingerprint") || title.contains("AddFingerprint"))
            return "👆";
        return "•";
    }
    
    QString getDescription(const QString &title)
    {
        if (title.contains("Add") || title.contains("AddPerson"))
            return "Add new person to database";
        else if (title.contains("Import"))
            return "Import person records from file";
        else if (title.contains("Inquery") || title.contains("Query") || title.contains("Search"))
            return "Search and view person records";
        else if (title.contains("Fingerprint") || title.contains("AddFingerprint"))
            return "Add fingerprint to person record";
        return "People management";
    }

private:
    QString m_title;
    QString m_iconPath;
    QLabel *m_descLabel;
};

class ManagingPeopleFrmPrivate
{
    Q_DECLARE_PUBLIC(ManagingPeopleFrm)
public:
    ManagingPeopleFrmPrivate(ManagingPeopleFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    QString getIconPath(const QString &title);
private:
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<PeopleManagementCardWidget*> m_cardWidgets;
private:
    ManagingPeopleFrm *const q_ptr;
};

ManagingPeopleFrmPrivate::ManagingPeopleFrmPrivate(ManagingPeopleFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void ManagingPeopleFrmPrivate::InitUI()
{
    Q_Q(ManagingPeopleFrm);
    
    // Set main widget object name for styling
    q->setObjectName("PeopleManagementMainWidget");
    
    // Main layout - NO HEADER
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(0);
    
    // Scroll area for cards - fills entire screen
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("PeopleManagementScrollArea");
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("PeopleManagementContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(12);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add scroll area to main layout - it takes all available space
    m_pMainLayout->addWidget(m_pScrollArea, 1);
}

QString ManagingPeopleFrmPrivate::getIconPath(const QString &title)
{
    // Define your PNG icon paths here
    if (title.contains("Add") || title.contains("AddPerson"))
        return ":/Images/add_person_icon.png";
    else if (title.contains("Import"))
        return ":/Images/import_person_icon.png";
    else if (title.contains("Inquery") || title.contains("Query") || title.contains("Search"))
        return ":/Images/search_person_icon.png";
    else if (title.contains("Fingerprint") || title.contains("AddFingerprint"))
        return ":/Images/fingerprint_icon.png";
    return ":/Images/people_icon.png";
}

void ManagingPeopleFrmPrivate::InitData()
{
    LanguageFrm::GetInstance()->UseLanguage(0);
    
    QStringList listName;
    // Added AddFingerprint option along with InqueryPerson
    listName << QObject::tr("InqueryPerson") << QObject::tr("AddFingerprint");
    
    for(int i = 0; i < listName.count(); i++)
    {
        QString title = listName.at(i);
        QString iconPath = getIconPath(title);
        PeopleManagementCardWidget *cardWidget = new PeopleManagementCardWidget(title, iconPath);
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    // Add stretch at the end to push cards to top
    m_pContentLayout->addStretch();
}

void ManagingPeopleFrmPrivate::InitConnect()
{
    for(PeopleManagementCardWidget *card : m_cardWidgets)
    {
        QObject::connect(card, &PeopleManagementCardWidget::cardClicked, 
                        [this](const QString &title) {
            Q_Q(ManagingPeopleFrm);
            q->handleCardClicked(title);
        });
    }
}

// Rest of your existing ManagingPeopleFrm implementation...
ManagingPeopleFrm::ManagingPeopleFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new ManagingPeopleFrmPrivate(this))
{
    // Ensure the main widget uses full available space
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ManagingPeopleFrm::~ManagingPeopleFrm()
{
}

void ManagingPeopleFrm::handleCardClicked(const QString &title)
{
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");
#endif
    
    emit sigShowFrm(title);
}

void ManagingPeopleFrm::slotIemClicked(QListWidgetItem *item)
{
    // Keep for compatibility - functionality moved to handleCardClicked
    // Since we've moved to card-based UI, this method is no longer used
    // but kept for compatibility. The actual functionality is now in handleCardClicked.
    
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");
#endif
    
    // For compatibility, we'll just emit a generic signal
    // In practice, this method won't be called since we're using cards now
    emit sigShowFrm(QObject::tr("InqueryPerson"));
}

#ifdef SCREENCAPTURE
void ManagingPeopleFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");
}
#endif

#include "ManagingPeopleFrm.moc" // For PeopleManagementCardWidget Q_OBJECT