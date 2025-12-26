// Modified AboutMachineFrm.cpp - Card-based theme implementation

#include "AboutMachineFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "OperationTipsFrm.h"
#include "SystemMaintenanceFrm.h"
#include "DevicePoweroffFrm.h"
#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "Version.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"
#include "../SettingEditTextFrm.h"
#include "USB/UsbObserver.h"
#include "HttpServer/ConnHttpServerThread.h"

#include <QTimer>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>

// Custom About Machine Card Widget
class AboutMachineCardWidget : public QFrame
{
    Q_OBJECT
public:
    enum CardType { InfoCard, ToggleCard, ActionCard };
    
    AboutMachineCardWidget(const QString &title, CardType type, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_cardType(type)
    {
        setObjectName("AboutCardWidget");
        setupUI(title, type);
    }
    
    QString getTitle() const { return m_title; }
    
    void setValue(const QString &value)
    {
        if (m_cardType == InfoCard && m_valueLabel) {
            m_valueLabel->setText(value);
        }
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
    
    void setDescription(const QString &description)
    {
        if (m_descLabel) {
            m_descLabel->setText(description);
        }
    }

signals:
    void cardClicked(const QString &title);
    void toggleChanged(bool enabled);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (m_cardType != InfoCard) {
            emit cardClicked(m_title);
        }
        QFrame::mousePressEvent(event);
    }
    
    void enterEvent(QEvent *event) override
    {
        if (m_cardType != InfoCard) {
            setProperty("hovered", true);
            style()->unpolish(this);
            style()->polish(this);
        }
        QFrame::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override
    {
        if (m_cardType != InfoCard) {
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
    mainLayout->setContentsMargins(12, 8, 12, 8); // Reduced from (12, 10, 12, 10)
    mainLayout->setSpacing(10); // Reduced from 12
    
    // Icon container - make smaller
    QFrame *iconFrame = new QFrame;
    iconFrame->setFixedSize(20, 20); // Reduced from (24, 24)
    iconFrame->setObjectName(getIconObjectName(title));
    
    QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *iconLabel = new QLabel;
    iconLabel->setText(getIconText(title));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setObjectName("AboutIconLabel");
    iconLayout->addWidget(iconLabel);
    
    // Text container - reduce spacing
    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setSpacing(2); // Reduced from 3
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("AboutTitleLabel");
    textLayout->addWidget(titleLabel);
    
    if (type == InfoCard) {
        m_valueLabel = new QLabel("--");
        m_valueLabel->setObjectName("AboutValueLabel");
        textLayout->addWidget(m_valueLabel);
    } else if (type == ActionCard) {
        m_descLabel = new QLabel(getDescription(title));
        m_descLabel->setObjectName("AboutDescLabel");
        textLayout->addWidget(m_descLabel);
    }
    
    // Right side control - reduce spacing
    QHBoxLayout *rightLayout = new QHBoxLayout;
    rightLayout->setSpacing(8); // Reduced from 10
    
    if (type == ToggleCard) {
        m_toggleCheckBox = new QCheckBox;
        m_toggleCheckBox->setObjectName("AboutToggleBox");
        QObject::connect(m_toggleCheckBox, &QCheckBox::toggled, this, &AboutMachineCardWidget::toggleChanged);
        rightLayout->addWidget(m_toggleCheckBox);
    } else if (type == ActionCard) {
        QLabel *arrowLabel = new QLabel("›");
        arrowLabel->setObjectName("AboutArrowLabel");
        rightLayout->addWidget(arrowLabel);
    }
    
    // Main layout assembly
    mainLayout->addWidget(iconFrame);
    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);
}
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Version") || title.contains("SerianNo") || title.contains("algo"))
            return "InfoIconFrame";
        else if (title.contains("debug"))
            return "DeveloperIconFrame";
        else if (title.contains("Screen") || title.contains("Touch"))
            return "DisplayIconFrame";
        else if (title.contains("Upgrade"))
            return "UpdateIconFrame";
        else if (title.contains("Maintenance"))
            return "MaintenanceIconFrame";
        else if (title.contains("Shutdown"))
            return "PowerIconFrame";
        else if (title.contains("Return") || title.contains("Factory"))
            return "ResetIconFrame";
        return "DefaultAboutIconFrame";
    }
    
    QString getIconText(const QString &title)
    {
        if (title.contains("Version"))
            return "";
        else if (title.contains("SerianNo"))
            return "";
        else if (title.contains("algo"))
            return "";
        else if (title.contains("debug"))
            return "";
        else if (title.contains("Screen"))
            return "";
        else if (title.contains("Touch"))
            return "";
        else if (title.contains("Upgrade"))
            return "⬆";
        else if (title.contains("Maintenance"))
            return "";
        else if (title.contains("Shutdown"))
            return "";
        else if (title.contains("ReturnSetting"))
            return "";
        else if (title.contains("Factory"))
            return "";
        return "";
    }
    
    QString getDescription(const QString &title)
    {
        if (title.contains("Screen"))
            return "Configure screen parameters";
        else if (title.contains("Touch"))
            return "Touch screen configuration";
        else if (title.contains("Upgrade"))
            return "Download and install firmware";
        else if (title.contains("Maintenance"))
            return "System maintenance settings";
        else if (title.contains("Shutdown"))
            return "Power off the device";
        else if (title.contains("ReturnSetting"))
            return "Reset all settings";
        else if (title.contains("Factory"))
            return "Reset to factory defaults";
        return "Device operation";
    }

private:
    QString m_title;
    CardType m_cardType;
    QLabel *m_valueLabel = nullptr;
    QLabel *m_descLabel = nullptr;
    QCheckBox *m_toggleCheckBox = nullptr;
};

class AboutMachineFrmPrivate
{
    Q_DECLARE_PUBLIC(AboutMachineFrm)
public:
    AboutMachineFrmPrivate(AboutMachineFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    bool CheckTopWidget();
private:
    QVBoxLayout *m_pMainLayout;
 //   QLabel *m_pHeaderLabel;
  //  QFrame *m_pHeaderFrame;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<AboutMachineCardWidget*> m_cardWidgets;
private:
    AboutMachineFrm *const q_ptr;
};

AboutMachineFrmPrivate::AboutMachineFrmPrivate(AboutMachineFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void AboutMachineFrmPrivate::InitUI()
{
    Q_Q(AboutMachineFrm);
    
    // Set main widget object name for styling
    q->setObjectName("AboutMachineMainWidget");
    
    // Main layout - reduce margins and spacing
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(12, 8, 12, 8); // Reduced from (12, 12, 12, 12)
    m_pMainLayout->setSpacing(12); // Reduced from 20

    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("AboutMachineScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("AboutMachineContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(4); // Reduced from 6
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout
    m_pMainLayout->addWidget(m_pScrollArea);
}

void AboutMachineFrmPrivate::InitData()
{
    // Create cards for each setting
    QStringList cardTitles = {
        QObject::tr("Verson"),              // Index 0 - Info
        QObject::tr("SerianNo"),            // Index 1 - Info
        QObject::tr("algoVersion"),         // Index 2 - Info
        QObject::tr("debugMode"),           // Index 3 - Toggle
        QObject::tr("ScreenParam"),         // Index 4 - Action
        QObject::tr("TouchScreen"),         // Index 5 - Action
      //  QObject::tr("SystemUpgrade"),       // Index 6 - Action
        QObject::tr("SystemMaintenance"),   // Index 7 - Action
        QObject::tr("Shutdown"),            // Index 8 - Action
        QObject::tr("ReturnSetting"),       // Index 9 - Action
        QObject::tr("ReturnToFactorySetting") // Index 10 - Action
    };
    
    for(int i = 0; i < cardTitles.count(); i++)
    {
        AboutMachineCardWidget::CardType type;
        if (i < 3) {
            type = AboutMachineCardWidget::InfoCard;
        } else if (i == 3) {
            type = AboutMachineCardWidget::ToggleCard;
        } else {
            type = AboutMachineCardWidget::ActionCard;
        }
        
        AboutMachineCardWidget *cardWidget = new AboutMachineCardWidget(cardTitles.at(i), type);
        
        // Set initial values
        switch(i) {
            case 0: // Version
                cardWidget->setValue(ISC_VERSION);
                break;
            case 1: // Serial Number
                cardWidget->setValue("Loading...");
                break;
            case 2: // Algorithm Version
                cardWidget->setValue(QObject::tr("Samay"));
                break;
            case 3: // Debug Mode
                cardWidget->setToggleState(ReadConfig::GetInstance()->getDebugMode_Value());
                break;
        }
        
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    // Add stretch at the end
    m_pContentLayout->addStretch();
}

void AboutMachineFrmPrivate::InitConnect()
{
    for(int i = 0; i < m_cardWidgets.count(); i++)
    {
        AboutMachineCardWidget *card = m_cardWidgets[i];
        
        if (i == 3) { // Debug Mode toggle
            QObject::connect(card, &AboutMachineCardWidget::toggleChanged, 
                           [this](bool enabled) {
                ReadConfig::GetInstance()->setDebugMode_Value(enabled ? 1 : 0);
            });
        } else if (i > 3) { // Action cards
            QObject::connect(card, &AboutMachineCardWidget::cardClicked, 
                           [this, i](const QString &title) {
                Q_Q(AboutMachineFrm);
                q->handleCardClicked(i);
            });
        }
    }
}

bool AboutMachineFrmPrivate::CheckTopWidget()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog") return true;
    }
    return false;
}

// Rest of your existing AboutMachineFrm implementation...
AboutMachineFrm::AboutMachineFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new AboutMachineFrmPrivate(this))
{
}

AboutMachineFrm::~AboutMachineFrm()
{
}

void AboutMachineFrm::setEnter()
{
    Q_D(AboutMachineFrm);
    
    // Update card values when entering the screen
    if (d->m_cardWidgets.count() >= 4) {
        // Serial Number (Index 1)
        d->m_cardWidgets[1]->setValue(myHelper::getCpuSerial());
        // Algorithm Version (Index 2) - already set to "Samay"
        // Debug Mode (Index 3)
        d->m_cardWidgets[3]->setToggleState(ReadConfig::GetInstance()->getDebugMode_Value());
    }
}

void AboutMachineFrm::setLeaveEvent()
{
    Q_D(AboutMachineFrm);
    
    // Debug mode is automatically saved via toggle connection
    printf(">>>%s,%s,%d,getCheckBoxState=%d\n",__FILE__ , __func__,__LINE__, 
           d->m_cardWidgets[3]->getToggleState());
    ReadConfig::GetInstance()->setDebugMode_Value(d->m_cardWidgets[3]->getToggleState() ? 1 : 0);
    
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog") {
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }

    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void AboutMachineFrm::handleCardClicked(int cardIndex)
{
    Q_D(AboutMachineFrm);
    
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
#endif 

    switch(cardIndex) {
        case 4: // Screen Parameters
        {
            SettingEditTextFrm dlg(this);
            dlg.setTitle(QObject::tr("TouchScreen"));
            QFile file("/param/RV1109_PARAM.txt");        
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray array;
                while (file.atEnd() == false) {
                    array += file.readLine();
                }
                dlg.setData(array);
            }        
            dlg.move((QApplication::desktop()->screenGeometry().width()-dlg.width())/2, 
                    (QApplication::desktop()->screenGeometry().height()-dlg.height())/2);
            dlg.exec();
            break;
        }
        case 5: // Touch Screen
        {
            SettingEditTextFrm dlg(this);
            dlg.setTitle(QObject::tr("TouchScreen"));
            QString str = "";
            FILE *pFile = popen("cat /proc/gt9xx_config", "r");
            if (pFile) {
                std::string ret = "";
                char buf[3072-32-64-436-8] = { 0 };
                int readSize = 0;
                do {
                    readSize = fread(buf, 1, sizeof(buf), pFile);
                    if (readSize > 0) {
                        ret += std::string(buf, 0, readSize);
                    }
                } while (0);
                pclose(pFile);
                str = QString::fromStdString(ret);
                dlg.setData(str);          
            }
            dlg.move((QApplication::desktop()->screenGeometry().width()-dlg.width())/2, 
                    (QApplication::desktop()->screenGeometry().height()-dlg.height())/2);
            dlg.exec();  
            break;
        }
        case 6: // System Maintenance
        {
            SystemMaintenanceFrm dlg(this);
            dlg.setData(ReadConfig::GetInstance()->getMaintenance_boot(), 
                       ReadConfig::GetInstance()->getMaintenance_bootTimer());
            if(dlg.exec() == 0) {
                ReadConfig::GetInstance()->setMaintenance_boot(dlg.getTimeMode());
                ReadConfig::GetInstance()->setMaintenance_bootTimer(dlg.getTime());
            }
            break;
        }
        case 7: // Shutdown
        {
            DevicePoweroffFrm dlg(this);
            dlg.exec();
            break;
        }
        case 8: // Return Settings
        {
            OperationTipsFrm dlg(this);
            int ret = dlg.setMessageBox(QObject::tr("ReturnSetting"), 
                                       QObject::tr("ReturnSettingHint"));
            if(ret == 0) {
#ifdef Q_OS_LINUX
                system("rm -rf /mnt/user/parameters.ini");
                myHelper::Utils_Reboot();
#endif
            }
            break;
        }
        case 9: // Factory Reset
        {
            OperationTipsFrm dlg(this);
            int ret = dlg.setMessageBox(QObject::tr("ReturnToFactorySetting"), 
                                       QObject::tr("ReturnToFactorySettingHint"));
            if(ret == 0) {
#ifdef Q_OS_LINUX
                system("rm -rf /mnt/user/parameters.ini");
                system("rm -rf /mnt/user/facedb/*");
                system("rm -rf /mnt/user/face_crop_image/*");
                myHelper::Utils_Reboot();
#endif
            }
            break;
        }
    }
}

void AboutMachineFrm::slotIemClicked(QListWidgetItem *item)
{
    // Keep for compatibility - functionality moved to handleCardClicked
}

#ifdef SCREENCAPTURE
void AboutMachineFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "AboutMachineFrm.moc" // For AboutMachineCardWidget Q_OBJECT