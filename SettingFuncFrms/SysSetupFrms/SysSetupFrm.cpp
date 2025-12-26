// Modified SysSetupFrm.cpp - Card-based theme implementation

#include "SysSetupFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "LanguageFrm.h"
#include "FillLightFrm.h"
#include "LuminanceFrm.h"
#include "VolumeFrm.h"
#include "LogPasswordChangeFrm.h"
#include "QRCodeFrm.h"
#include "CompareFingerFrm.h"
#include "SyncFunctionality.h"
#include "Helper/myhelper.h"
#include "Config/ReadConfig.h"
#include "OperationTipsFrm.h"
#include "DeviceInfo.h"
#include "DeleteAllFingerprintsFrm.h"
#include "USB/UsbObserver.h"
#include "HttpServer/ConnHttpServerThread.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QScroller>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QListWidgetItem>
#include <QtConcurrent/QtConcurrent>
#include <QNetworkInterface>
#include <QTimer>
#include <QFileInfo>

// Custom System Setting Card Widget
class SystemSettingCardWidget : public QFrame
{
    Q_OBJECT
public:
    enum CardType { ValueCard, ActionCard };
    
    SystemSettingCardWidget(const QString &title, CardType type, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_cardType(type)
    {
        setObjectName("SystemCardWidget");
        setupUI(title, type);
    }
    
    QString getTitle() const { return m_title; }
    
    void setValue(const QString &value)
    {
        if (m_cardType == ValueCard && m_valueLabel) {
            m_valueLabel->setText(value);
        }
    }
    
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
    void setupUI(const QString &title, CardType type)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(16, 12, 16, 12);
        mainLayout->setSpacing(12);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(24, 24);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
     QLabel *iconLabel = new QLabel;
    iconLabel->setText(""); // Remove all icon text
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setObjectName("SystemIconLabel");
    iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("SystemTitleLabel");
        textLayout->addWidget(titleLabel);
        
        if (type == ValueCard) {
            m_valueLabel = new QLabel("--");
            m_valueLabel->setObjectName("SystemValueLabel");
            textLayout->addWidget(m_valueLabel);
        } else {
            m_descLabel = new QLabel(getDescription(title));
            m_descLabel->setObjectName("SystemDescLabel");
            textLayout->addWidget(m_descLabel);
        }
        
        // Arrow
        QLabel *arrowLabel = new QLabel("›");
        arrowLabel->setObjectName("SystemArrowLabel");
        
        // Main layout assembly
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(arrowLabel);
    }
    
    QString getIconObjectName(const QString &title)
{
    // Return same frame type for all to get uniform color
    return "DefaultSystemIconFrame";
}
    
    QString getIconText(const QString &title)
    {
        if (title.contains("Language"))
            return "";
        else if (title.contains("Brightness"))
            return "";
        else if (title.contains("Fill"))
            return "";
        else if (title.contains("Volume"))
            return "";
        else if (title.contains("Storage"))
            return "";
        else if (title.contains("CompareFinger"))
             return "";
        else if (title.contains("DeleteAllFinger"))
             return "";        
        else if (title.contains("Password"))
            return "";
        else if (title.contains("SystemUpgrade") || title.contains("Upgrade"))
            return "";
        else if (title.contains("About"))
            return "";
        else if (title.contains("QR"))
            return "";
        else if (title.contains("Sync"))
            return "";
        return "";
    }
    
    QString getDescription(const QString &title)
    {
        if (title.contains("SystemUpgrade") || title.contains("Upgrade"))
            return "Download and install firmware";
        else if (title.contains("Storage"))
            return "View storage information";
        else if (title.contains("Password"))
            return "Change login password";
        else if (title.contains("About"))
            return "Device information";
        else if (title.contains("QR"))
            return "Enroll via QR code";
        else if (title.contains("Sync"))
            return "Synchronize devices";
        return "System setting";
    }

private:
    QString m_title;
    CardType m_cardType;
    QLabel *m_valueLabel = nullptr;
    QLabel *m_descLabel = nullptr;
};

class SysSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(SysSetupFrm)
public:
    SysSetupFrmPrivate(SysSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    bool CheckTopState();
private:
    QVBoxLayout *m_pMainLayout;
  //  QLabel *m_pHeaderLabel;
   // QFrame *m_pHeaderFrame;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<SystemSettingCardWidget*> m_cardWidgets;
private:
    LanguageFrm *m_pLanguageFrm;
    LuminanceFrm *m_pLuminanceFrm;
    FillLightFrm *m_pFillLightFrm;
    VolumeFrm *m_pVolumeFrm;
    LogPasswordChangeFrm *m_pLogPasswordChangeFrm;
    QRCodeFrm *m_pQRCodeFrm;
    DeleteAllFingerprintsFrm *m_pDeleteAllFingerprintsFrm;
    CompareFingerFrm *m_pCompareFingerFrm;
private:
    SysSetupFrm *const q_ptr;
};

SysSetupFrmPrivate::SysSetupFrmPrivate(SysSetupFrm *dd)
    : q_ptr(dd)
    , m_pLanguageFrm(nullptr)
    , m_pLuminanceFrm(nullptr)
    , m_pFillLightFrm(nullptr)
    , m_pVolumeFrm(nullptr)
    , m_pLogPasswordChangeFrm(nullptr)
    , m_pQRCodeFrm(nullptr)
    , m_pDeleteAllFingerprintsFrm(nullptr)
    , m_pCompareFingerFrm(nullptr)  // Add this line
{
    qDebug() << "SysSetupFrmPrivate constructor START";
    
    try {
        qDebug() << "Creating dialog forms...";
        m_pLanguageFrm = new LanguageFrm(q_ptr);
        m_pLuminanceFrm = new LuminanceFrm(q_ptr);
        m_pFillLightFrm = new FillLightFrm(q_ptr);
        m_pVolumeFrm = new VolumeFrm(q_ptr);
        m_pLogPasswordChangeFrm = new LogPasswordChangeFrm(q_ptr);
        m_pQRCodeFrm = new QRCodeFrm(q_ptr);
        m_pDeleteAllFingerprintsFrm = new DeleteAllFingerprintsFrm(q_ptr);
        m_pCompareFingerFrm = new CompareFingerFrm(q_ptr);
        qDebug() << "All dialog forms created successfully";
        
        this->InitUI();
        this->InitData();
        this->InitConnect();
        
    } catch (...) {
        qDebug() << "EXCEPTION in SysSetupFrmPrivate constructor!";
        throw;
    }
    
    qDebug() << "SysSetupFrmPrivate constructor END";
}

void SysSetupFrmPrivate::InitUI()
{
    Q_Q(SysSetupFrm);
    
    // Set main widget object name for styling
    q->setObjectName("SysSetupMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(20);
    
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("SysSetupScrollArea");
    
    // Disable scroll bars
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("SysSetupContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(6);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout
   // m_pMainLayout->addWidget(m_pHeaderFrame);
    m_pMainLayout->addWidget(m_pScrollArea);
}

void SysSetupFrmPrivate::InitData()
{
    QStringList cardTitles = {
    QObject::tr("LanguageSettings"),    // Index 0 - Value
    QObject::tr("BrightnessSetting"),   // Index 1 - Value
    QObject::tr("FillLightSetting"),    // Index 2 - Value
    QObject::tr("VolumeSetting"),       // Index 3 - Value
    QObject::tr("QR Enrollment"),       // Index 4 - Action
    QObject::tr("StorageCapacity"),     // Index 5 - Action
    QObject::tr("LoginPassword"),       // Index 6 - Action
    QObject::tr("SystemUpgrade"),       // Index 7 - Action (NEW)
    QObject::tr("About"),               // Index 8 - Action (was 7)
    QObject::tr("Sync Devices") ,        // Index 9 - Action (was 8)
    QObject::tr("CompareFingerprint"),
    QObject::tr("DeleteAllFinger")
};
    
    for(int i = 0; i < cardTitles.count(); i++)
    {
        SystemSettingCardWidget::CardType type = (i < 4) ? 
            SystemSettingCardWidget::ValueCard : 
            SystemSettingCardWidget::ActionCard;
            
        SystemSettingCardWidget *cardWidget = new SystemSettingCardWidget(cardTitles.at(i), type);
        
        // Set initial values for value cards
        if (type == SystemSettingCardWidget::ValueCard) {
            switch(i) {
                case 0: // Language
                    cardWidget->setValue(QObject::tr("English"));
                    break;
                case 1: // Brightness
                    cardWidget->setValue("10");
                    break;
                case 2: // Fill Light
                    cardWidget->setValue(QObject::tr("Auto"));
                    break;
                case 3: // Volume
                    cardWidget->setValue("8");
                    break;
            }
        }
        
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    // Add stretch at the end
    m_pContentLayout->addStretch();
}

void SysSetupFrmPrivate::InitConnect()
{
    for(int i = 0; i < m_cardWidgets.count(); i++)
    {
        SystemSettingCardWidget *card = m_cardWidgets[i];
        QObject::connect(card, &SystemSettingCardWidget::cardClicked, 
                        [this, i](const QString &title) {
            Q_Q(SysSetupFrm);
            q->handleCardClicked(i);
        });
    }
}

bool SysSetupFrmPrivate::CheckTopState()
{
    if(!m_pLuminanceFrm->isHidden())
    {
        if (m_cardWidgets.count() > 1) {
            m_cardWidgets[1]->setValue(QString::number(m_pLuminanceFrm->getAdjustValue()));
        }
        ReadConfig::GetInstance()->setLuminance_Value(m_pLuminanceFrm->getAdjustValue());
        m_pLuminanceFrm->hide();
        return true;
    }
    else if(!m_pVolumeFrm->isHidden())
    {
        if (m_cardWidgets.count() > 3) {
            m_cardWidgets[3]->setValue(QString::number(m_pVolumeFrm->getAdjustValue()));
        }
        m_pVolumeFrm->hide();
        return true;
    }
    return false;
}

static QString LanguageIdexToSTR(const int &index)
{
    switch(index)
    {
    case 0:return QObject::tr("English");
    case 1:return QObject::tr("English");
    }
    return QObject::tr("English");
}

static QString FillLightIndexToSTR(const int &index)
{
    switch(index)
    {
        case 0:return QObject::tr("NormallyClosed");
        case 1:return QObject::tr("NormallyOpen");
        case 2:return QObject::tr("Auto");
    }
    return QObject::tr("Auto");
}

// Rest of your existing SysSetupFrm implementation...
SysSetupFrm::SysSetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new SysSetupFrmPrivate(this))
{
}

SysSetupFrm::~SysSetupFrm()
{
}

void SysSetupFrm::setEnter()
{
    Q_D(SysSetupFrm);
    
    qDebug() << "=== setEnter() DEBUG START ===";
    
    if (!d || !ReadConfig::GetInstance()) {
        qDebug() << "FATAL: d_ptr or ReadConfig is NULL!";
        return;
    }

    ReadConfig* config = ReadConfig::GetInstance();
    
    try {
        // Set form values
        if (d->m_pLanguageFrm) d->m_pLanguageFrm->setLanguageMode(config->getLanguage_Mode());
        if (d->m_pLuminanceFrm) d->m_pLuminanceFrm->setAdjustValue(config->getLuminance_Value());
        if (d->m_pFillLightFrm) d->m_pFillLightFrm->setFillLightMode(config->getFillLight_Value());
        if (d->m_pVolumeFrm) d->m_pVolumeFrm->setAdjustValue(config->getVolume_Value());

        // Update card display values
        if (d->m_cardWidgets.count() >= 4) {
            d->m_cardWidgets[0]->setValue(LanguageIdexToSTR(config->getLanguage_Mode()));
            d->m_cardWidgets[1]->setValue(QString::number(config->getLuminance_Value()));
            d->m_cardWidgets[2]->setValue(FillLightIndexToSTR(config->getFillLight_Value()));
            d->m_cardWidgets[3]->setValue(QString::number(config->getVolume_Value()));
        }
        
        qDebug() << "All UI updates completed successfully";
        
    } catch (...) {
        qDebug() << "Exception caught in setEnter()";
    }
    
    qDebug() << "=== setEnter() DEBUG END ===";
}

void SysSetupFrm::setLeaveEvent()
{
    Q_D(SysSetupFrm);
    d->CheckTopState();
    if(!d->m_pLuminanceFrm->isHidden()) d->m_pLuminanceFrm->hide();
    if(!d->m_pVolumeFrm->isHidden()) d->m_pVolumeFrm->hide();
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void SysSetupFrm::handleCardClicked(int cardIndex)
{
    Q_D(SysSetupFrm);
    
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");  
#endif 

    int width = QApplication::desktop()->screenGeometry().width();    
    if(d->CheckTopState()) {
        return;
    }

    switch(cardIndex) {
        case 0: // Language Settings
        {
            d->m_pLanguageFrm->exec();
            ReadConfig::GetInstance()->setLanguage_Mode(d->m_pLanguageFrm->getLanguageMode());
            d->m_cardWidgets[0]->setValue(LanguageIdexToSTR(ReadConfig::GetInstance()->getLanguage_Mode()));
            break;
        }
        case 1: // Brightness Settings
        {
            d->m_pLuminanceFrm->show();
            break;
        }
        case 2: // Fill Light Settings
        {
            d->m_pFillLightFrm->exec();
            ReadConfig::GetInstance()->setFillLight_Value(d->m_pFillLightFrm->getFillLightMode());
            d->m_cardWidgets[2]->setValue(FillLightIndexToSTR(ReadConfig::GetInstance()->getFillLight_Value()));
            break;
        }
        case 3: // Volume Settings
        {
            d->m_pVolumeFrm->show();
            break;
        }
        case 4: // QR Code Enrollment
        {
            QString serialNumber = DeviceInfo::getInstance()->getSerialNumber();
            d->m_pQRCodeFrm->setSerialNumber();
            d->m_pQRCodeFrm->exec();
            break;
        }
        case 5: // Storage Capacity
        {
            emit sigShowFrm(d->m_cardWidgets[cardIndex]->getTitle());
            break;
        }
        case 6: // Login Password
        {
            if (width == 480)
                d->m_pLogPasswordChangeFrm->setFixedSize(d->m_pLogPasswordChangeFrm->width()-40, d->m_pLogPasswordChangeFrm->height());
            d->m_pLogPasswordChangeFrm->show();
            int ret = d->m_pLogPasswordChangeFrm->exec();
            if(ret == 0)
                ReadConfig::GetInstance()->setLoginPassword(d->m_pLogPasswordChangeFrm->getNewPasw());
            break;
        }
        case 7: // System Upgrade (NEW)
        {
            OperationTipsFrm dlg(this);
            int ret = dlg.setMessageBox(
                QObject::tr("SystemUpgrade"), 
                QObject::tr("Download and install latest firmware?\nDevice will reboot after update."), 
                QObject::tr("Update Now"),
                QObject::tr("Cancel"),
                1
            );
            if(ret == 0) {
                performFirmwareUpdate();
            }
            break;
        }
        case 8: // About
        {
            emit sigShowFrm(d->m_cardWidgets[cardIndex]->getTitle());
            break;
        }
        case 9: // Sync Devices
        {
            SyncFunctionality::ShowSyncDialog();
            break;
        }
    }
}

// Keep existing firmware update methods
void SysSetupFrm::performFirmwareUpdate()
{
    qDebug() << "=== FIRMWARE UPDATE START (Using doDownloadFile) ===";
    
    showUpdateProgress(QObject::tr("Preparing for firmware update..."));
    
    QTimer::singleShot(1000, [this]() {
        qDebug() << "Step 1: Cleaning up previous files...";
        
        myHelper::Utils_ExecCmd("rm -rf /update/*");
        myHelper::Utils_ExecCmd("rm -rf /mnt/user/update.zip");
        myHelper::Utils_ExecCmd("mkdir -p /update/lost+found");
        myHelper::Utils_ExecCmd("sync");
        
        qDebug() << "Step 1 completed.";
        
        QTimer::singleShot(500, [this]() {
            qDebug() << "Step 2: Starting download using doDownloadFile method...";
            showUpdateProgress(QObject::tr("Downloading firmware from server..."));
            
            QTimer::singleShot(1000, [this]() {
                qDebug() << "Step 2: Executing download...";
                
                QString firmwareUrl = "https://pub-21c70f63cead42d3895d45cffa96b65d.r2.dev/tenant_001_data%20(1)/update.zip";
                QString downloadPath = "/mnt/user/update.zip";
                
                qDebug() << "Downloading from:" << firmwareUrl;
                qDebug() << "Downloading to:" << downloadPath;
                
                downloadFirmwareFile(firmwareUrl, downloadPath);
            });
        });
    });
}

void SysSetupFrm::downloadFirmwareFile(const QString &url, const QString &destPath)
{
    qDebug() << "Starting download using integrated method...";
    
    QString deviceSN = myHelper::getCpuSerial();
    QString fullUrl = url + "?sn=" + deviceSN;
    
    qDebug() << "Full download URL:" << fullUrl;
    
    QString downloadCmd = QString(
        "/usr/bin/curl --cacert /isc/cacert.pem "
        "--connect-timeout 30 --max-time 300 "
        "-o %1 '%2'"
    ).arg(destPath).arg(fullUrl);
    
    qDebug() << "Executing download command:" << downloadCmd;
    
    FILE *pFile = popen(downloadCmd.toStdString().c_str(), "r");
    if (pFile != nullptr) {
        char buf[4096] = { 0 };
        QString output = "";
        int readSize = 0;
        do {
            readSize = fread(buf, 1, sizeof(buf), pFile);
            if (readSize > 0) {
                output += QString::fromStdString(std::string(buf, 0, readSize));
            }
        } while (readSize > 0);
        pclose(pFile);
        
        qDebug() << "Download output:" << output;
    }
    
    continueAfterDownload();
}

void SysSetupFrm::continueAfterDownload()
{
    qDebug() << "Step 3: Checking download result...";
    
    QFileInfo fileInfo("/mnt/user/update.zip");
    if (fileInfo.exists() && fileInfo.size() > 1000) {
        qDebug() << "Download successful! File size:" << fileInfo.size() << "bytes";
        showUpdateProgress(QObject::tr("Download completed. Validating firmware..."));
        
        QTimer::singleShot(1000, [this]() {
            qDebug() << "Step 4: Starting validation...";
            
            bool isValid = UsbObserver::GetInstance()->doCheckUpdateImage("/mnt/user/update.zip");
            qDebug() << "Validation result:" << (isValid ? "PASSED" : "FAILED");
            
            if (isValid) {
                showUpdateProgress(QObject::tr("Firmware validated. Installing update..."));
                
                QTimer::singleShot(2000, [this]() {
                    showUpdateProgress(QObject::tr("Installing firmware... Device will reboot."));
                    
                    QTimer::singleShot(1000, []() {
                        qDebug() << "Step 5: Starting firmware installation...";
                        UsbObserver::GetInstance()->doDeviceUpdate();
                    });
                });
            } else {
                qDebug() << "Validation failed!";
                showUpdateProgress(QObject::tr("Firmware validation failed!"));
                
                QTimer::singleShot(2000, [this]() {
                    OperationTipsFrm errorDlg(this);
                    errorDlg.setMessageBox(
                        QObject::tr("Update Failed"), 
                        QObject::tr("Firmware validation failed. Please try again."), 
                        QObject::tr("Ok"), "", 1
                    );
                });
            }
        });
    } else {
        qDebug() << "Download failed - file does not exist or is too small";
        if (fileInfo.exists()) {
            qDebug() << "File exists but size is only:" << fileInfo.size() << "bytes";
        }
        showUpdateProgress(QObject::tr("Download failed!"));
        
        QTimer::singleShot(2000, [this]() {
            OperationTipsFrm errorDlg(this);
            errorDlg.setMessageBox(
                QObject::tr("Download Failed"), 
                QObject::tr("Failed to download firmware. Check network connection."), 
                QObject::tr("Ok"), "", 1
            );
        });
    }
}

void SysSetupFrm::showUpdateProgress(const QString &message)
{
    qDebug() << "Update Progress:" << message;
    
    static OperationTipsFrm *progressDlg = nullptr;
    
    if (progressDlg) {
        progressDlg->close();
        delete progressDlg;
    }
    
    progressDlg = new OperationTipsFrm(this);
    progressDlg->setMessageBox(
        QObject::tr("System Update"), 
        message, 
        "", "", 2
    );
    progressDlg->show();
    QApplication::processEvents();
    
    if (!message.contains("Installing") && !message.contains("reboot")) {
        QTimer::singleShot(3000, [progressDlg]() {
            if (progressDlg) {
                progressDlg->close();
            }
        });
    }
}

void SysSetupFrm::slotIemClicked(QListWidgetItem *item)
{
    // Keep for compatibility - functionality moved to handleCardClicked
}

#ifdef SCREENCAPTURE
void SysSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "SysSetupFrm.moc" // For SystemSettingCardWidget Q_OBJECT