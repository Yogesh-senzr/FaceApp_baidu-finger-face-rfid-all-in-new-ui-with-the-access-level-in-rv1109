#include "Network4GViewFrm.h"
#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"
#include "Helper/myhelper.h"
#include "OperationTipsFrm.h"

#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QDesktopWidget>

// Custom 4G Network Card Widget
class Network4GCardWidget : public QFrame
{
    Q_OBJECT
public:
    enum CardType { 
        ToggleCard      // Only 4G On/Off toggle remains
    };
    
    Network4GCardWidget(const QString &title, CardType type, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_cardType(type)
    {
        setObjectName("Network4GCardWidget");
        setupUI(title, type);
    }
    
    void setToggleState(bool enabled)
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            m_toggleCheckBox->setChecked(enabled);
        }
    }
    
    bool getToggleState() const
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            return m_toggleCheckBox->isChecked();
        }
        return false;
    }

signals:
    void toggleChanged(int state);

protected:
    void enterEvent(QEvent *event) override
    {
        if (m_cardType == ToggleCard) {
            setProperty("hovered", true);
            style()->unpolish(this);
            style()->polish(this);
        }
        QFrame::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override
    {
        if (m_cardType == ToggleCard) {
            setProperty("hovered", false);
            style()->unpolish(this);
            style()->polish(this);
        }
        QFrame::leaveEvent(event);
    }

private:
    void setupUI(const QString &title, CardType type)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(40, 40);
        iconFrame->setObjectName("Mobile4GIconFrame");
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setText("📶");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("Network4GIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("Network4GTitleLabel");
        textLayout->addWidget(titleLabel);
        
        QLabel *descLabel = new QLabel(tr("Enable 4G/LTE cellular connection"));
        descLabel->setObjectName("Network4GDescLabel");
        textLayout->addWidget(descLabel);
        
        // Toggle checkbox
        m_toggleCheckBox = new QCheckBox;
        m_toggleCheckBox->setObjectName("Network4GToggleBox");
        QObject::connect(m_toggleCheckBox, QOverload<int>::of(&QCheckBox::stateChanged),
                       this, &Network4GCardWidget::toggleChanged);
        
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(m_toggleCheckBox);
    }

private:
    QString m_title;
    CardType m_cardType;
    QCheckBox *m_toggleCheckBox = nullptr;
};

class Network4GViewFrmPrivate
{
    Q_DECLARE_PUBLIC(Network4GViewFrm)
public:
    Network4GViewFrmPrivate(Network4GViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    
    Network4GCardWidget *m_pToggleCard;
    
private:
    Network4GViewFrm *const q_ptr;
};

Network4GViewFrmPrivate::Network4GViewFrmPrivate(Network4GViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void Network4GViewFrmPrivate::InitUI()
{
    Q_Q(Network4GViewFrm);
    
    // Set main widget object name for styling
    q->setObjectName("Network4GViewMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(20);
    
    // Scroll area for the toggle card
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("Network4GViewScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("Network4GViewContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(12);
    
    // Create only the toggle card
    m_pToggleCard = new Network4GCardWidget(QObject::tr("4G"), Network4GCardWidget::ToggleCard);
    
    // Add only the toggle card to layout
    m_pContentLayout->addWidget(m_pToggleCard);
    m_pContentLayout->addStretch();
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout (no header frame)
    m_pMainLayout->addWidget(m_pScrollArea);
}

void Network4GViewFrmPrivate::InitData()
{
    // Set initial state for toggle card only
    bool is4GEnabled = (ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3);
    m_pToggleCard->setToggleState(is4GEnabled);
}

void Network4GViewFrmPrivate::InitConnect()
{
    Q_Q(Network4GViewFrm);
    
    // Connect only the toggle card
    QObject::connect(m_pToggleCard, &Network4GCardWidget::toggleChanged,
                    q, &Network4GViewFrm::slotNetwork4GSwitchState);
}

Network4GViewFrm::Network4GViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new Network4GViewFrmPrivate(this))
{
}

Network4GViewFrm::~Network4GViewFrm()
{
}

void Network4GViewFrm::slotNetwork4GSwitchState(const int state)
{
    Q_D(Network4GViewFrm);
    
    LogD("%s %s[%d] state %d visible %d \n",__FILE__,__FUNCTION__,__LINE__,state,this->isVisible());
    
    if(state == 2 && this->isVisible())
    {
        OperationTipsFrm dlg(this);
        int ret = dlg.setMessageBox(QObject::tr("Reboot"), 
                                   QObject::tr("Enabling 4G network requires device restart. Continue?"));
        if (ret == 0)
        {
            ReadConfig::GetInstance()->setNetwork_Manager_Mode(3);
            ReadConfig::GetInstance()->setSaveConfig();
            LogD("%s %s[%d] Mode=%d  \n",__FILE__,__FUNCTION__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
            
            myHelper::Utils_Reboot();
        }
        else
        {
            // User cancelled, revert toggle state
            d->m_pToggleCard->setToggleState(false);
        }
        emit sigShowLevelFrm();
    }
    else if(state == 0)
    {
        // 4G disabled
        ReadConfig::GetInstance()->setNetwork_Manager_Mode(1); // Back to Ethernet mode
        ReadConfig::GetInstance()->setSaveConfig();
    }
}

void Network4GViewFrm::setEnter()
{
    Q_D(Network4GViewFrm);
    
    // Update toggle state when entering the screen
    bool is4GEnabled = (ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3);
    d->m_pToggleCard->setToggleState(is4GEnabled);
}

void Network4GViewFrm::setLeaveEvent()
{
    // Exit cleanup if needed
}

#ifdef SCREENCAPTURE
void Network4GViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "Network4GViewFrm.moc" // For Network4GCardWidget Q_OBJECT