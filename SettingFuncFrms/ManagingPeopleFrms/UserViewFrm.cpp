#include "UserViewFrm.h"
#include "Delegate/wholedelegate.h"
#include "pagenavigator.h"

#ifdef Q_OS_LINUX
#include "USB/UsbObserver.h"
#endif

#include "UserViewModel.h"
#include "DB/RegisteredFacesDB.h"
#include "OperationTipsFrm.h"
#include "Threads/ParsePersonXlsx.h"
#include "Application/FaceApp.h"

#include "Helper/myhelper.h"
#include "AddUserFrm.h"
#include "FaceHomeFrms/FaceHomeFrm.h"
#include "FaceMainFrm.h"
#include "HttpServer/ConnHttpServerThread.h"

#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlDriver>
#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <QListWidget>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QAction>
#include <QStackedWidget>
#include <QFrame>
#include <QScrollArea>
#include <QGraphicsEffect>

#define PAGE (10) // 10 users per page
#define DateTime QDateTime::currentDateTime().toString("yyyy-MM-dd")

struct UserData {
    QString name;
    QString sex;
    QString iccardnum;
    QString idcardnum;
    QString createtime;
    QString personid;
    QString uuid;
    bool hasFaceData;
};

// Custom User Card Widget
class UserCardWidget : public QFrame
{
    Q_OBJECT
public:
    UserCardWidget(const UserData &userData, QWidget *parent = nullptr) 
        : QFrame(parent), m_userData(userData)
    {
        setObjectName("UserCard");
        setupUI();
        setUserData(userData);
    }
    
    void setUserData(const UserData &userData) {
        m_userData = userData;
        updateDisplay();
    }
    
    const UserData& getUserData() const { return m_userData; }

signals:
    void cardClicked(const UserData &userData);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        emit cardClicked(m_userData);
    }
    
    void enterEvent(QEvent *event) override {
        Q_UNUSED(event);
        setProperty("hovered", true);
        style()->unpolish(this);
        style()->polish(this);
    }
    
    void leaveEvent(QEvent *event) override {
        Q_UNUSED(event);
        setProperty("hovered", false);
        style()->unpolish(this);
        style()->polish(this);
    }

private:
    void setupUI() {
        setFixedHeight(80);
        setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 15, 20, 15);
        mainLayout->setSpacing(15);
        
        // Avatar
        m_avatarLabel = new QLabel;
        m_avatarLabel->setObjectName("UserAvatar");
        m_avatarLabel->setFixedSize(50, 50);
        m_avatarLabel->setAlignment(Qt::AlignCenter);
        
        // User info container
        QWidget *infoWidget = new QWidget;
        QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
        infoLayout->setContentsMargins(0, 0, 0, 0);
        infoLayout->setSpacing(4);
        
        m_nameLabel = new QLabel;
        m_nameLabel->setObjectName("UserName");
        
        m_idLabel = new QLabel;
        m_idLabel->setObjectName("UserId");
        
        infoLayout->addWidget(m_nameLabel);
        infoLayout->addWidget(m_idLabel);
        
        // Arrow button
        m_arrowButton = new QPushButton;
        m_arrowButton->setObjectName("UserArrowButton");
        m_arrowButton->setText(">");
        m_arrowButton->setFixedSize(36, 36);
        m_arrowButton->setCursor(Qt::PointingHandCursor);
        
        connect(m_arrowButton, &QPushButton::clicked, [this]() {
            emit cardClicked(m_userData);
        });
        
        mainLayout->addWidget(m_avatarLabel);
        mainLayout->addWidget(infoWidget, 1);
        mainLayout->addWidget(m_arrowButton);
    }
    
    void updateDisplay() {
        // Generate avatar initials
        QString initials;
        if (!m_userData.name.isEmpty()) {
            QStringList words = m_userData.name.split(' ', QString::SkipEmptyParts);
            for (const QString &word : words) {
                if (!word.isEmpty()) {
                    initials += word.at(0).toUpper();
                    if (initials.length() >= 2) break;
                }
            }
            if (initials.isEmpty()) {
                initials = m_userData.name.left(1).toUpper();
            }
        } else {
            initials = "U";
        }
        
        m_avatarLabel->setText(initials);
        m_nameLabel->setText(m_userData.name);
        m_idLabel->setText(QString("EMP ID: %1").arg(m_userData.idcardnum));
    }
    
private:
    UserData m_userData;
    QLabel *m_avatarLabel;
    QLabel *m_nameLabel;
    QLabel *m_idLabel;
    QPushButton *m_arrowButton;
};

class UserViewFrmPrivate
{
    Q_DECLARE_PUBLIC(UserViewFrm)
public:
    UserViewFrmPrivate(UserViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
    enum FilterOption {
        All,
        WithFace,
        NoFace
    };
private:
    void SelectData(const QString &sql);
    void ApplyCurrentFilter();
    void LoadFaceDataInBackground();   
private:
    void ClearListData();
    void ShowUserDetails(int index);
    void CreateDetailView();
    void UpdateUserGrid();
    QWidget* createDetailCard(const QString &labelText);
private:
    class FaceHomeFrm *m_pFaceHomeFrm;
private:
    class QStackedWidget *m_pStackedWidget;     
private:
    class QLineEdit *m_pInputDataEdit;
    class QPushButton *m_pQueryButton;
    class QPushButton *m_pExportButton;
    class QPushButton *m_pFilterButton;
    class QMenu *m_pFilterMenu;
    class QAction *m_pFilterAllAction;
    class QAction *m_pFilterFaceAction;
    class QAction *m_pFilterNoFaceAction;
    FilterOption m_currentFilterOption;
    class QProgressDialog *m_pProgressDialog; 
private:
    class QScrollArea *m_pScrollArea;
    class QWidget *m_pGridContainer;
    class QGridLayout *m_pGridLayout;
    class QWidget *m_pDetailView;
    class QLabel *m_pDetailNameLabel;
    class QLabel *m_pDetailSexLabel;
    class QLabel *m_pDetailCardNoLabel;
    class QLabel *m_pDetailIdCardLabel;
    class QLabel *m_pDetailCreateTimeLabel;
    class QLabel *m_pDetailPersonIdLabel;
    class QLabel *m_pDetailUuidLabel;
    class QLabel *m_pDetailFaceDataLabel;
    class QLabel *m_pDetailAvatarLabel;
    class QLabel *m_pDetailIdLabel;
    class QPushButton *m_pAddFaceButton;
    class QPushButton *m_pEditFaceButton; 
    class QPushButton *m_pBackButton;
    class QPushButton *m_pRefreshButton; 
    
    // Controls and navigation widgets
    class QWidget *m_pControlsWidget;
    class QWidget *m_pBottomWidget; 

private:
    QList<struct UserData> m_userDataList;
    QList<struct UserData> m_filteredUserDataList;
    QList<UserCardWidget*> m_cardWidgets;
    int m_currentDetailIndex;
private:
    class PageNavigator *m_pPageNavigator;
    class QLabel *m_pUserCountLabel;
private:
    UserViewFrm *const q_ptr;
};

UserViewFrmPrivate::UserViewFrmPrivate(UserViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

UserViewFrm::UserViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new UserViewFrmPrivate(this))
{

}

UserViewFrm::~UserViewFrm()
{

}

void UserViewFrmPrivate::InitUI()
{
    // Header and controls
    m_pInputDataEdit = new QLineEdit; 
    m_pQueryButton = new QPushButton; 
    m_pExportButton = new QPushButton; 
    m_pFilterButton = new QPushButton; 
    
    // Filter menu setup
    m_pFilterMenu = new QMenu(m_pFilterButton);
    QFont filterFont;
    filterFont.setPointSize(7);

    m_pFilterAllAction = m_pFilterMenu->addAction(QObject::tr("All Data"));
    m_pFilterAllAction->setFont(filterFont);
    m_pFilterFaceAction = m_pFilterMenu->addAction(QObject::tr("Face Data"));
    m_pFilterFaceAction->setFont(filterFont);
    m_pFilterNoFaceAction = m_pFilterMenu->addAction(QObject::tr("No Face"));
    m_pFilterNoFaceAction->setFont(filterFont);

    m_pFilterButton->setMenu(m_pFilterMenu);
    
    m_pPageNavigator = new PageNavigator;
    m_pPageNavigator->setObjectName("PaginationContainer");
    
   // m_pUserCountLabel = new QLabel(QObject::tr("Total Users: 0"));
   // m_pUserCountLabel->setObjectName("UserCountModern");
   // m_pUserCountLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Controls widget (can be hidden)
    m_pControlsWidget = new QWidget;
    QHBoxLayout *controlsLayout = new QHBoxLayout(m_pControlsWidget);
    controlsLayout->setSpacing(15);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addWidget(m_pInputDataEdit, 1);
    controlsLayout->addWidget(m_pQueryButton);
    controlsLayout->addWidget(m_pFilterButton);
    controlsLayout->addWidget(m_pExportButton);

// Bottom widget - INCREASED HEIGHT
m_pBottomWidget = new QWidget;
m_pBottomWidget->setFixedHeight(70); // Increased height
m_pBottomWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

QHBoxLayout *bottomLayout = new QHBoxLayout(m_pBottomWidget);
bottomLayout->setContentsMargins(20, 5, 20, 5); // Reduced vertical margins
bottomLayout->setSpacing(0);

// Add the page navigator to use full width
bottomLayout->addWidget(m_pPageNavigator);
    // Main grid container with scroll area
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_pGridContainer = new QWidget;
    m_pGridLayout = new QGridLayout(m_pGridContainer);
    m_pGridLayout->setSpacing(15);
    m_pGridLayout->setContentsMargins(30, 20, 30, 20);
    
    m_pScrollArea->setWidget(m_pGridContainer);

    // Detail view
    m_pDetailView = new QWidget;
    CreateDetailView();

    // Stacked widget
    m_pStackedWidget = new QStackedWidget;
    m_pStackedWidget->addWidget(m_pScrollArea);
    m_pStackedWidget->addWidget(m_pDetailView);

    // Main layout - UPDATED MARGINS
    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(20, 0, 20, 25); // Increased bottom margin

    vlayout->addWidget(m_pControlsWidget);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_pStackedWidget, 1);
    vlayout->addSpacing(10); // Add spacing before bottom widget
    vlayout->addWidget(m_pBottomWidget);
    vlayout->addSpacing(5); // Add final bottom padding
}

void UserViewFrmPrivate::ApplyCardStyles()
{
    // Main container styles
    q_func()->setStyleSheet(
        "UserViewFrm {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #667eea, stop:1 #764ba2);"
        "}"
        
        // Input field
        "QLineEdit {"
        "    padding: 12px 16px;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    background: white;"
        "    max-width: 300px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2196F3;"
        "}"
        
        // Buttons
        "QPushButton {"
        "    padding: 12px 24px;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    transform: translateY(-1px);"
        "}"
        
        // Scroll area
        "QScrollArea {"
        "    background: white;"
        "    border-radius: 15px;"
        "    border: none;"
        "}"
        
        // User cards
        "QFrame#UserCard {"
        "    background: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #e1e5e9;"
        "}"
        "QFrame#UserCard:hover {"
        "    border-color: #2196F3;"
        "    background: #fafafa;"
        "}"
        "QFrame#UserCard[hovered=\"true\"] {"
        "    border-color: #2196F3;"
        "    background: #fafafa;"
        "}"
        
        // Avatar
        "QLabel#UserAvatar {"
        "    background: #2c3e50;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 18px;"
        "    border-radius: 25px;"
        "}"
        
        // User name
        "QLabel#UserName {"
        "    font-size: 18px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "}"
        
        // User ID
        "QLabel#UserId {"
        "    font-size: 14px;"
        "    color: #6c757d;"
        "    font-weight: 700;"
        "}"
        
        // Arrow button
        "QPushButton#UserArrowButton {"
        "    background: #f8f9fa;"
        "    color: #6c757d;"
        "    border-radius: 18px;"
        "    font-size: 16px;"
        "    min-width: 36px;"
        "    max-width: 36px;"
        "}"
        "QPushButton#UserArrowButton:hover {"
        "    background: #2196F3;"
        "    color: white;"
        "}"
        
        // Detail view styles
        "QWidget#DetailHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "}"
        "QLabel#DetailTitle {"
        "    color: white;"
        "    font-size: 24px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#RefreshButton {"
        "    background: rgba(255, 255, 255, 0.2);"
        "    border: none;"
        "    border-radius: 24px;"
        "    min-width: 48px;"
        "    max-width: 48px;"
        "}"
        "QPushButton#RefreshButton:hover {"
        "    background: rgba(255, 255, 255, 0.3);"
        "    transform: rotate(180deg);"
        "}"
        "QPushButton#BackButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "}"
        "QPushButton#BackButton:hover {"
        "    background: #1976D2;"
        "}"
        
        // Large avatar
        "QLabel#DetailAvatar {"
        "    background: #2c3e50;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 36px;"
        "    border-radius: 45px;"
        "}"
        
        // Large name and ID
        "QLabel#DetailNameLarge {"
        "    font-size: 36px;"
        "    font-weight: 700;"
        "    color: #2c3e50;"
        "}"
        "QLabel#DetailIdLarge {"
        "    font-size: 18px;"
        "    color: #6c757d;"
        "    font-weight: 700;"
        "}"
        
        // Detail cards - UPDATED FOR SINGLE COLUMN WITH HOVER EFFECTS
    "QWidget#DetailCard {"
    "    background: #f8f9fa;"
    "    border-radius: 12px;"
    "    border-left: 4px solid #2196F3;"
    "    min-height: 70px;" // Reduced from 80px
    "    padding: 20px;" // Reduced from 25px
    "}"
    "QWidget#DetailCard:hover {"
    "    background: #ffffff;"
    "    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);"
    "    transform: translateY(-2px);"
    "}"
        "QLabel#DetailCardLabel {"
        "    font-size: 14px;"
        "    color: #6c757d;"
        "    font-weight: 600;"
        "    text-transform: uppercase;"
        "}"
        "QLabel#DetailValue {"
    "    font-size: 16px;" // Slightly reduced from 18px
    "    color: #2c3e50;"
    "    font-weight: 600;"
    "    line-height: 1.4;" // Better line spacing for wrapped text
    "}"
        
        // Action buttons
        "QPushButton#AddFaceButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    border-radius: 10px;"
        "    min-width: 180px;"
        "}"
        "QPushButton#AddFaceButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#AddFaceButton:disabled {"
        "    background: #9e9e9e;"
        "}"
        "QPushButton#EditFaceButton {"
        "    background: #FF9800;"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    border-radius: 10px;"
        "    min-width: 180px;"
        "}"
        "QPushButton#EditFaceButton:hover {"
        "    background: #F57C00;"
        "}"
        "QPushButton#EditFaceButton:disabled {"
        "    background: #9e9e9e;"
        "}"
        
        // MODERN PAGINATION STYLES
        "QWidget#PaginationContainer {"
"    background: rgba(255, 255, 255, 0.95);"
"    border-radius: 20px;"
"    border: 1px solid rgba(33, 150, 243, 0.15);"
"    min-width: 400px;" // Ensure minimum width
"}"
        "QPushButton#PaginationButton {"
        "    background: transparent;"
        "    border: 1px solid #e1e5e9;"
        "    color: #6c757d;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    min-width: 32px;"
        "    min-height: 32px;"
        "    max-width: 32px;"
        "    max-height: 32px;"
        "    border-radius: 16px;"
        "    margin: 1px;"
        "}"
        "QPushButton#PaginationButton:hover {"
        "    background: #f8f9fa;"
        "    border-color: #2196F3;"
        "    color: #2196F3;"
        "}"
        "QPushButton#PaginationButtonActive {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border: 1px solid #2196F3;"
        "    color: white;"
        "    font-size: 12px;"
        "    font-weight: 700;"
        "    min-width: 32px;"
        "    min-height: 32px;"
        "    max-width: 32px;"
        "    max-height: 32px;"
        "    border-radius: 16px;"
        "    margin: 1px;"
        "}"
        "QPushButton#PaginationArrow {"
        "    background: #f8f9fa;"
        "    border: 1px solid #e1e5e9;"
        "    color: #6c757d;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 36px;"
        "    min-height: 32px;"
        "    max-width: 36px;"
        "    max-height: 32px;"
        "    border-radius: 16px;"
        "    margin: 1px;"
        "}"
        "QPushButton#PaginationArrow:hover {"
        "    background: #2196F3;"
        "    border-color: #2196F3;"
        "    color: white;"
        "}"
        "QPushButton#PaginationArrow:disabled {"
        "    background: #f1f3f4;"
        "    border-color: #e8eaed;"
        "    color: #dadce0;"
        "}"
        "QLabel#PageInfoLabel {"
        "    color: #546e7a;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    padding: 6px 10px;"
        "    background: rgba(84, 110, 122, 0.1);"
        "    border-radius: 12px;"
        "    border: 1px solid rgba(84, 110, 122, 0.15);"
        "}"
    );
    
    // Set specific button styles
    m_pQueryButton->setStyleSheet("background: #2196F3; color: white;");
    m_pFilterButton->setStyleSheet("background: #6c757d; color: white;");
    m_pExportButton->setStyleSheet("background: #28a745; color: white;");
}

void UserViewFrmPrivate::CreateDetailView()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(m_pDetailView);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Header section
    QWidget *headerWidget = new QWidget;
    headerWidget->setObjectName("DetailHeader");
    headerWidget->setFixedHeight(100);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(30, 0, 30, 0);
    
    QLabel *titleLabel = new QLabel(QObject::tr("User Details"));
    titleLabel->setObjectName("DetailTitle");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    m_pRefreshButton = new QPushButton;
    m_pRefreshButton->setObjectName("RefreshButton");
    m_pRefreshButton->setFixedSize(48, 48);
    m_pRefreshButton->setToolTip(QObject::tr("Refresh User Data"));
    
    // Set PNG icon instead of text
    QIcon refreshIcon(":/icons/refresh.png");
    m_pRefreshButton->setIcon(refreshIcon);
    m_pRefreshButton->setIconSize(QSize(24, 24));
    m_pRefreshButton->setText("");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_pRefreshButton);

    // Back button section
    QWidget *backSection = new QWidget;
    QHBoxLayout *backLayout = new QHBoxLayout(backSection);
    backLayout->setContentsMargins(30, 25, 30, 0);
    
    m_pBackButton = new QPushButton(QObject::tr("← Back to User List"));
    m_pBackButton->setObjectName("BackButton");
    m_pBackButton->setFixedHeight(45);
    m_pBackButton->setMinimumWidth(200);
    
    backLayout->addWidget(m_pBackButton);
    backLayout->addStretch();

    // Profile section - REDUCED SPACING
    QWidget *profileWidget = new QWidget;
    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);
    profileLayout->setContentsMargins(30, 25, 30, 30); // Reduced top margin from 40 to 25
    profileLayout->setSpacing(25); // Reduced spacing from 40 to 25

    // Profile header with SMALLER avatar
    QWidget *profileHeaderWidget = new QWidget;
    QHBoxLayout *profileHeaderLayout = new QHBoxLayout(profileHeaderWidget);
    profileHeaderLayout->setSpacing(25); // Reduced spacing
    
    // SMALLER avatar - reduced from 120x120 to 90x90
    m_pDetailAvatarLabel = new QLabel;
    m_pDetailAvatarLabel->setObjectName("DetailAvatar");
    m_pDetailAvatarLabel->setFixedSize(90, 90); // REDUCED SIZE
    m_pDetailAvatarLabel->setAlignment(Qt::AlignCenter);
    
    // Profile info
    QWidget *profileInfoWidget = new QWidget;
    QVBoxLayout *profileInfoLayout = new QVBoxLayout(profileInfoWidget);
    profileInfoLayout->setSpacing(6); // Reduced spacing
    profileInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    m_pDetailNameLabel = new QLabel();
    m_pDetailNameLabel->setObjectName("DetailNameLarge");
    
    m_pDetailIdLabel = new QLabel();
    m_pDetailIdLabel->setObjectName("DetailIdLarge");
    
    profileInfoLayout->addWidget(m_pDetailNameLabel);
    profileInfoLayout->addWidget(m_pDetailIdLabel);
    profileInfoLayout->addStretch();
    
    profileHeaderLayout->addWidget(m_pDetailAvatarLabel);
    profileHeaderLayout->addWidget(profileInfoWidget, 1);

    // SINGLE COLUMN DETAILS LAYOUT - MOVED UP
    QWidget *detailsWidget = new QWidget;
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setSpacing(15); // Reduced spacing from 20 to 15
    detailsLayout->setContentsMargins(0, 0, 0, 0);

    // Create detail cards in single column
    QWidget *sexCard = createDetailCard(QObject::tr("Gender"));
    m_pDetailSexLabel = sexCard->findChild<QLabel*>("DetailValue");
    
    QWidget *cardNoCard = createDetailCard(QObject::tr("Card Number"));
    m_pDetailCardNoLabel = cardNoCard->findChild<QLabel*>("DetailValue");
    
    QWidget *idCardCard = createDetailCard(QObject::tr("ID Card"));
    m_pDetailIdCardLabel = idCardCard->findChild<QLabel*>("DetailValue");
    
    QWidget *createTimeCard = createDetailCard(QObject::tr("Created Date"));
    m_pDetailCreateTimeLabel = createTimeCard->findChild<QLabel*>("DetailValue");
    
    QWidget *personIdCard = createDetailCard(QObject::tr("Person ID"));
    m_pDetailPersonIdLabel = personIdCard->findChild<QLabel*>("DetailValue");
    
    QWidget *faceDataCard = createDetailCard(QObject::tr("Face Data Status"));
    m_pDetailFaceDataLabel = faceDataCard->findChild<QLabel*>("DetailValue");
    
    // SPECIAL HANDLING FOR UUID CARD - FULL WIDTH WITH WORD WRAP
    QWidget *uuidCard = createDetailCard(QObject::tr("UUID"));
    m_pDetailUuidLabel = uuidCard->findChild<QLabel*>("DetailValue");
    // Enable word wrapping for UUID to handle long text
    m_pDetailUuidLabel->setWordWrap(true);
    m_pDetailUuidLabel->setMinimumHeight(60); // Ensure enough height for wrapped text

    // Add all cards to single column layout
    detailsLayout->addWidget(sexCard);
    detailsLayout->addWidget(cardNoCard);
    detailsLayout->addWidget(idCardCard);
    detailsLayout->addWidget(createTimeCard);
    detailsLayout->addWidget(personIdCard);
    detailsLayout->addWidget(faceDataCard);
    detailsLayout->addWidget(uuidCard);

    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setSpacing(20);
    actionsLayout->setContentsMargins(0, 15, 0, 0); // Reduced top margin from 20 to 15

    m_pAddFaceButton = new QPushButton(QObject::tr("Add Face Data"));
    m_pAddFaceButton->setObjectName("AddFaceButton");
    m_pAddFaceButton->setFixedHeight(50);
    m_pAddFaceButton->setMinimumWidth(180);

    m_pEditFaceButton = new QPushButton(QObject::tr("Edit Face Data"));
    m_pEditFaceButton->setObjectName("EditFaceButton");
    m_pEditFaceButton->setFixedHeight(50);
    m_pEditFaceButton->setMinimumWidth(180);

    actionsLayout->addStretch();
    actionsLayout->addWidget(m_pAddFaceButton);
    actionsLayout->addWidget(m_pEditFaceButton);
    actionsLayout->addStretch();

    // Add all sections to profile layout
    profileLayout->addWidget(profileHeaderWidget);
    profileLayout->addWidget(detailsWidget);
    profileLayout->addWidget(actionsWidget);
    profileLayout->addStretch();

    // Add all sections to main layout
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(backSection);
    mainLayout->addWidget(profileWidget, 1);
}


QWidget* UserViewFrmPrivate::createDetailCard(const QString &labelText)
{
    QWidget *card = new QWidget;
    card->setObjectName("DetailCard");
    
    // Special handling for UUID card - make it taller
    if (labelText == QObject::tr("UUID")) {
        card->setMinimumHeight(100); // Increased height for UUID
    } else {
        card->setMinimumHeight(70); // Slightly reduced for other cards
    }
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 20, 20, 20); // Slightly reduced padding
    cardLayout->setSpacing(6); // Reduced spacing
    
    QLabel *titleLabel = new QLabel(labelText);
    titleLabel->setObjectName("DetailCardLabel");
    
    QLabel *valueLabel = new QLabel;
    valueLabel->setObjectName("DetailValue");
    
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);
    cardLayout->addStretch();
    
    return card;
}

void UserViewFrmPrivate::InitData()
{
    m_pQueryButton->setText(QObject::tr("Search")); 
    m_pExportButton->setText(QObject::tr("Export")); 
    m_pFilterButton->setText(QObject::tr("Filter")); 

    m_pInputDataEdit->setPlaceholderText(QObject::tr("Search by Name, Card Number, or ID Card..."));
    
    m_currentFilterOption = FilterOption::All;
    m_pFilterAllAction->setData("selected");
}

void UserViewFrmPrivate::InitConnect()
{
    QObject::connect(m_pQueryButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotQueryButton);
    QObject::connect(m_pPageNavigator, &PageNavigator::currentPageChanged, q_func(), &UserViewFrm::slotCurrentPageChanged);
    QObject::connect(m_pExportButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotExportButton);
    QObject::connect(m_pBackButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotBackToListView);
    QObject::connect(m_pAddFaceButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotAddFaceButton);
    QObject::connect(m_pEditFaceButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotEditFaceButton);
    
    // Connect filter actions
    QObject::connect(m_pFilterAllAction, SIGNAL(triggered()), 
                    q_func(), SLOT(slotFilterActionTriggered()));
    QObject::connect(m_pFilterFaceAction, SIGNAL(triggered()), 
                    q_func(), SLOT(slotFilterActionTriggered()));
    QObject::connect(m_pFilterNoFaceAction, SIGNAL(triggered()), 
                    q_func(), SLOT(slotFilterActionTriggered()));
    QObject::connect(m_pRefreshButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotRefreshUserData);
}

void UserViewFrmPrivate::UpdateUserGrid()
{
    // Clear existing cards
    qDeleteAll(m_cardWidgets);
    m_cardWidgets.clear();
    
    // Clear grid layout
    QLayoutItem *item;
    while ((item = m_pGridLayout->takeAt(0)) != nullptr) {
        delete item;
    }

    int currentPage = m_pPageNavigator->getCurrentPage();
    int startIndex = (currentPage - 1) * PAGE;
    int endIndex = qMin(startIndex + PAGE, m_filteredUserDataList.size());

    // Single column layout
    int row = 0;

    for (int i = startIndex; i < endIndex; ++i) {
        UserCardWidget *card = new UserCardWidget(m_filteredUserDataList.at(i), m_pGridContainer);
        
        // Connect card click signal
        QObject::connect(card, &UserCardWidget::cardClicked, q_func(), 
                        [this](const UserData &userData) {
                            // Find original index
                            for (int j = 0; j < m_userDataList.size(); ++j) {
                                if (m_userDataList.at(j).uuid == userData.uuid) {
                                    ShowUserDetails(j);
                                    break;
                                }
                            }
                        });
        
        m_cardWidgets.append(card);
        // Single column: always column 0
        m_pGridLayout->addWidget(card, row, 0);
        row++;
    }
    
    // Add stretch to push cards to top
    m_pGridLayout->setRowStretch(row, 1);
    
    // Set column stretch to make cards fill width
    m_pGridLayout->setColumnStretch(0, 1);
}

void UserViewFrmPrivate::SelectData(const QString &sql)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    qDebug() << "=== SelectData START ===";

    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(sql);
    query.exec();

    m_userDataList.clear();

    while (query.next()) {
        UserData userData;
        userData.name = query.value("name").toString();
        userData.sex = query.value("Sex").toString();
        userData.iccardnum = query.value("iccardnum").toString();
        userData.idcardnum = query.value("idcardnum").toString();
        userData.createtime = query.value("createtime").toString();
        userData.personid = query.value("personid").toString();
        userData.uuid = query.value("uuid").toString();
        userData.hasFaceData = false; // Will be set during filtering if needed
        
        m_userDataList.append(userData);
    }

    ApplyCurrentFilter();
   // m_pUserCountLabel->setText(QObject::tr("Total Users: %1").arg(m_filteredUserDataList.size()));
    
    qDebug() << "TOTAL SelectData time:" << totalTimer.elapsed() << "ms";
}

void UserViewFrmPrivate::ApplyCurrentFilter()
{
    m_filteredUserDataList.clear();

    for (UserData &userData : m_userDataList) {
        bool shouldAdd = false;
        
        switch (m_currentFilterOption) {
            case FilterOption::All:
                shouldAdd = true;
                break;
            case FilterOption::WithFace:
            case FilterOption::NoFace:
                QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(userData.uuid);
                bool hasFaceData = (persons.size() > 0 && !persons[0].feature.isEmpty());
                userData.hasFaceData = hasFaceData;
                shouldAdd = (m_currentFilterOption == FilterOption::WithFace) ? hasFaceData : !hasFaceData;
                break;
        }
        
        if (shouldAdd) {
            m_filteredUserDataList.append(userData);
        }
    }

    int pageCount = m_filteredUserDataList.size() <= PAGE ? 1 : 
                   (m_filteredUserDataList.size() / PAGE + (m_filteredUserDataList.size() % PAGE != 0 ? 1 : 0));
    m_pPageNavigator->setMaxPage(pageCount);

    UpdateUserGrid();
    
    // Show controls and navigation when displaying list view
    m_pControlsWidget->show();
    m_pBottomWidget->show();
    
    m_pStackedWidget->setCurrentWidget(m_pScrollArea);
}

void UserViewFrmPrivate::ClearListData()
{
    qDeleteAll(m_cardWidgets);
    m_cardWidgets.clear();
    m_userDataList.clear();
    m_filteredUserDataList.clear();
}

void UserViewFrmPrivate::ShowUserDetails(int index)
{
    if (index < 0 || index >= m_userDataList.size())
        return;
    
    m_currentDetailIndex = index;
    UserData &userData = m_userDataList[index];
    
    if (!userData.hasFaceData) {
        QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(userData.uuid);
        userData.hasFaceData = (persons.size() > 0 && !persons[0].feature.isEmpty());
    }
    
    // Generate avatar initials for detail view
    QString initials;
    if (!userData.name.isEmpty()) {
        QStringList words = userData.name.split(' ', QString::SkipEmptyParts);
        for (const QString &word : words) {
            if (!word.isEmpty()) {
                initials += word.at(0).toUpper();
                if (initials.length() >= 2) break;
            }
        }
        if (initials.isEmpty()) {
            initials = userData.name.left(1).toUpper();
        }
    } else {
        initials = "U";
    }
    
    // Update detail view content
    m_pDetailAvatarLabel->setText(initials);
    m_pDetailNameLabel->setText(userData.name);
    m_pDetailIdLabel->setText(QString("Employee ID: %1").arg(userData.idcardnum));
    m_pDetailSexLabel->setText(userData.sex);
    m_pDetailCardNoLabel->setText(userData.iccardnum);
    m_pDetailIdCardLabel->setText(userData.idcardnum);
    m_pDetailCreateTimeLabel->setText(userData.createtime);
    m_pDetailPersonIdLabel->setText(userData.personid);
    m_pDetailUuidLabel->setText(userData.uuid); // Simple text, word wrap enabled
    
    // NEW FACE DATA STATUS INDICATOR - ICON BASED
    QString faceDataText;
    if (userData.hasFaceData) {
        faceDataText = "✓ Face Data Available";
        // Apply green styling to the label
        m_pDetailFaceDataLabel->setStyleSheet(
            "QLabel { color: #28a745; font-weight: 600; font-size: 16px; }"
        );
    } else {
        faceDataText = "✗ No Face Data";
        // Apply red styling to the label
        m_pDetailFaceDataLabel->setStyleSheet(
            "QLabel { color: #dc3545; font-weight: 600; font-size: 16px; }"
        );
    }
    m_pDetailFaceDataLabel->setText(faceDataText);

    // Update button states
    m_pAddFaceButton->setEnabled(!userData.hasFaceData);
    m_pEditFaceButton->setEnabled(userData.hasFaceData);
    
    // Hide controls and navigation when showing detail view
    m_pControlsWidget->hide();
    m_pBottomWidget->hide();
    
    m_pStackedWidget->setCurrentWidget(m_pDetailView);
}


// Rest of the slot implementations remain the same...

void UserViewFrm::slotRefreshUserData()
{
    Q_D(UserViewFrm);
    
    if (d->m_currentDetailIndex < 0 || d->m_currentDetailIndex >= d->m_userDataList.size())
        return;
    
    const UserData &userData = d->m_userDataList.at(d->m_currentDetailIndex);
    QString employeeId = userData.idcardnum;

    d->m_pRefreshButton->setEnabled(false);
    d->m_pRefreshButton->setToolTip(QObject::tr("Syncing..."));
    d->m_pDetailNameLabel->setText(userData.name + " (Syncing...)");

    ConnHttpServerThread* syncThread = ConnHttpServerThread::GetInstance();
    if (syncThread) {
        connect(syncThread, &ConnHttpServerThread::userSyncCompleted,
                this, &UserViewFrm::onUserSyncCompleted, Qt::UniqueConnection);
        syncThread->syncIndividualUser(employeeId);
    } else {
        d->m_pRefreshButton->setEnabled(true);
        d->m_pRefreshButton->setToolTip(QObject::tr("Refresh User Data"));
        d->m_pDetailNameLabel->setText(userData.name);
    }
}

void UserViewFrm::onUserSyncCompleted(const QString& employeeId, bool success)
{
    Q_D(UserViewFrm);
    
    d->m_pRefreshButton->setEnabled(true);
    d->m_pRefreshButton->setToolTip(QObject::tr("Refresh User Data"));
    
    if (success) {
        refreshCurrentUserDetails();
        QTimer::singleShot(1000, this, [this]() {
            OperationTipsFrm dlg(this);
            dlg.setMessageBox(QObject::tr("Success"), 
                              QObject::tr("User data refreshed successfully"), 
                              QObject::tr("Ok"), QString(), 1);
            dlg.exec();
        });
    } else {
        OperationTipsFrm dlg(this);
        dlg.setMessageBox(QObject::tr("Error"), 
                          QObject::tr("Failed to refresh user data"), 
                          QObject::tr("Ok"), QString(), 1);
        dlg.exec();

        if (d->m_currentDetailIndex >= 0 && d->m_currentDetailIndex < d->m_userDataList.size()) {
            const UserData &userData = d->m_userDataList.at(d->m_currentDetailIndex);
            d->m_pDetailNameLabel->setText(userData.name);
        }
    }
}

void UserViewFrm::refreshCurrentUserDetails()
{
    Q_D(UserViewFrm);
    
    if (d->m_currentDetailIndex < 0 || d->m_currentDetailIndex >= d->m_userDataList.size())
        return;
    
    const UserData &currentUser = d->m_userDataList.at(d->m_currentDetailIndex);
    QString uuid = currentUser.uuid;

    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT * FROM person WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (query.exec() && query.next()) {
        UserData updatedData;
        updatedData.name = query.value("name").toString();
        updatedData.sex = query.value("sex").toString();
        updatedData.iccardnum = query.value("iccardnum").toString();
        updatedData.idcardnum = query.value("idcardnum").toString();
        updatedData.createtime = query.value("createtime").toString();
        updatedData.personid = query.value("personid").toString();
        updatedData.uuid = query.value("uuid").toString();
        
        QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(uuid);
        updatedData.hasFaceData = (persons.size() > 0 && !persons[0].feature.isEmpty());
        
        d->m_userDataList[d->m_currentDetailIndex] = updatedData;
        d->ShowUserDetails(d->m_currentDetailIndex);
    }
}

void UserViewFrm::slotEditFaceButton()
{
    Q_D(UserViewFrm);
    
    if (d->m_currentDetailIndex < 0 || d->m_currentDetailIndex >= d->m_userDataList.size())
        return;
    
    const UserData &userData = d->m_userDataList.at(d->m_currentDetailIndex);
    
    if (!userData.hasFaceData) {
        OperationTipsFrm dlg(this);
        dlg.setMessageBox(QObject::tr("Warning"), 
                          QObject::tr("No face data found for this user. Please add face data first."), 
                          QObject::tr("Ok"), QString(), 1);
        dlg.exec();
        return;
    }

    AddUserFrm::GetInstance()->modifyRecord(userData.name, userData.idcardnum, userData.iccardnum, 
                                           userData.sex, userData.personid, userData.uuid);
    emit sigShowFrm(QObject::tr("ModifyPerson"));
}

void UserViewFrm::slotFilterActionTriggered()
{
    Q_D(UserViewFrm);
    
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    d->m_pFilterAllAction->setData(QVariant());
    d->m_pFilterFaceAction->setData(QVariant());
    d->m_pFilterNoFaceAction->setData(QVariant());

    if (action == d->m_pFilterAllAction) {
        d->m_currentFilterOption = UserViewFrmPrivate::FilterOption::All;
        d->m_pFilterAllAction->setData("selected");
    } else if (action == d->m_pFilterFaceAction) {
        d->m_currentFilterOption = UserViewFrmPrivate::FilterOption::WithFace;
        d->m_pFilterFaceAction->setData("selected");
    } else if (action == d->m_pFilterNoFaceAction) {
        d->m_currentFilterOption = UserViewFrmPrivate::FilterOption::NoFace;
        d->m_pFilterNoFaceAction->setData("selected");
    }

    slotQueryButton();
}

void UserViewFrm::slotBackToListView()
{
    Q_D(UserViewFrm);
    
    // Show controls and navigation when returning to list view
    d->m_pControlsWidget->show();
    d->m_pBottomWidget->show();
    
    d->m_pStackedWidget->setCurrentWidget(d->m_pScrollArea);
}

void UserViewFrm::slotAddFaceButton()
{
    Q_D(UserViewFrm);
    
    if (d->m_currentDetailIndex < 0 || d->m_currentDetailIndex >= d->m_userDataList.size())
        return;
    
    const UserData &userData = d->m_userDataList.at(d->m_currentDetailIndex);

    AddUserFrm::GetInstance()->modifyRecord(userData.name, userData.idcardnum, userData.iccardnum, 
                                           userData.sex, userData.personid, userData.uuid);
    emit sigShowFrm(QObject::tr("ModifyPerson"));
}

void UserViewFrm::slotUserItemClicked(int row)
{
    Q_UNUSED(row);
}

void UserViewFrm::slotCurrentPageChanged(const int page)
{
    Q_D(UserViewFrm);
    d->m_pQueryButton->setFocus();
    d->UpdateUserGrid();
}

void UserViewFrm::slotQueryButton()
{
    Q_D(UserViewFrm);

    d->ClearListData();

    QString text = d->m_pInputDataEdit->text();
    QString selSQL;
    
    if (text.isEmpty()) {
        selSQL = "SELECT * FROM person ORDER BY personid DESC";
    } else {
        selSQL = QString("SELECT * FROM person WHERE name LIKE '%%1%' OR "
                        "idcardnum LIKE '%%1%' OR iccardnum LIKE '%%1%' "
                        "ORDER BY personid DESC").arg(text);
    }

    d->SelectData(selSQL);
    d->m_pStackedWidget->setCurrentWidget(d->m_pScrollArea);
}

void UserViewFrm::setEnter()
{
    Q_D(UserViewFrm);
    d->m_pQueryButton->setFocus();
    static bool Init = false;
    if(!Init) {
        Init = true;
        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigExportProgressShell, 
                        this, &UserViewFrm::slotExportProgressShell);
        QObject::connect(this, &UserViewFrm::sigExportPersons, 
                        qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::slotExportPersons);
    }
    
    d->m_currentFilterOption = UserViewFrmPrivate::FilterOption::All;
    d->m_pFilterButton->setText(QObject::tr("Filter"));
    d->m_pFilterAllAction->setData("selected");
    d->m_pFilterFaceAction->setData(QVariant());
    d->m_pFilterNoFaceAction->setData(QVariant());
    
    this->slotQueryButton();
}

void UserViewFrm::slotUpdateUserList()
{
    slotQueryButton();
}

void UserViewFrm::slotExportButton()
{
    Q_D(UserViewFrm);
#ifdef Q_OS_LINUX
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState) {
        d->m_pExportButton->setEnabled(false);
        d->m_pExportButton->setText(QObject::tr("Export..."));
        d->m_pProgressDialog = new QProgressDialog(this);
        QFont font("YSong18030", 12);
        d->m_pProgressDialog->setFont(font);
        d->m_pProgressDialog->setWindowModality(Qt::WindowModal);
        d->m_pProgressDialog->setAttribute(Qt::WA_DeleteOnClose);        
        d->m_pProgressDialog->setMinimumDuration(5);
        d->m_pProgressDialog->setWindowTitle(QObject::tr("PlsWaiting"));
        d->m_pProgressDialog->setLabelText(QObject::tr("Export..."));
        d->m_pProgressDialog->setCancelButtonText(NULL);        
        emit sigExportPersons();
    } else {
        OperationTipsFrm dlg(this);
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("PlsInsertUDisk"), QObject::tr("Ok"), QString(), 1);       
    }
#else
    d->m_pExportButton->setEnabled(false);
    d->m_pExportButton->setText(QObject::tr("Export..."));
    emit sigExportPersons();
#endif
}

void UserViewFrm::slotExportProgressShell(const bool dealstate, const bool savestate, const int total, const int dealcnt)
{
    Q_D(UserViewFrm);
    Q_UNUSED(savestate);
    if(dealstate) {
        int icnt = 0;
        
        d->m_pProgressDialog->setModal(true);             
        d->m_pProgressDialog->show();        
        if (total < 20) {            
            icnt = 100 / total;
            d->m_pProgressDialog->setRange(0, 100);    
            d->m_pProgressDialog->setValue(dealcnt * icnt);
            QTime dieTime = QTime::currentTime().addMSecs(500);
            while(QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        } else {
            d->m_pProgressDialog->setRange(0, total);    
            d->m_pProgressDialog->setValue(dealcnt);                        
        }
                
        if (total == dealcnt) {                        
            d->m_pExportButton->setEnabled(true);
            d->m_pExportButton->setText(QObject::tr("Export"));           
            d->m_pProgressDialog->close();
        }
    }
}

void UserViewFrm::setModifyFlag(int value)
{  
    Q_D(UserViewFrm);   
    mModifyFlag = value;
    this->slotQueryButton();     
}

#ifdef SCREENCAPTURE
void UserViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "UserViewFrm.moc"