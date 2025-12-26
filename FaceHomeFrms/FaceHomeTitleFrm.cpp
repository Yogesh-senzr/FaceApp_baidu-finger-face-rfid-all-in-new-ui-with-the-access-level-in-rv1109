#include "FaceHomeTitleFrm.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>
#include <QDebug>

#include "BaseFace/BaseFaceManager.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "Helper/myhelper.h"
#include "FaceMainFrm.h"
#include "MessageHandler/Log.h"

class FaceHomeTitleFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceHomeTitleFrm)
public:
    FaceHomeTitleFrmPrivate(FaceHomeTitleFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void updateClockDisplay();

private:
    QLabel *m_pTitleLabel;
    QLabel *m_pClockLabel;
    QLabel *m_pNetPngLabel;
    QLabel *m_pNetStatusLabel;
    
    QTimer *m_timer;
    int mSec = 0;
    
    // Server time tracking
    QDateTime m_serverTime;
    QDateTime m_lastServerTimeUpdate;
    bool m_useServerTime;
    
private:
    FaceHomeTitleFrm *m_FaceHomeTitleFrm;
private:
    FaceHomeTitleFrm *const q_ptr;
};

FaceHomeTitleFrmPrivate::FaceHomeTitleFrmPrivate(FaceHomeTitleFrm *dd)
    : q_ptr(dd), m_useServerTime(false)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceHomeTitleFrm::FaceHomeTitleFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new FaceHomeTitleFrmPrivate(this))
{
}

FaceHomeTitleFrm::~FaceHomeTitleFrm()
{

}

void FaceHomeTitleFrmPrivate::InitUI()
{
    // Create main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(q_func());
    mainLayout->setContentsMargins(30, 15, 30, 15);
    mainLayout->setSpacing(20);
    
    // Title section
    m_pTitleLabel = new QLabel("eSSL Face Recognition System");
    m_pTitleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #1a202c;"
        "    background: transparent;"
        "}"
    );
    
    // Clock section
    m_pClockLabel = new QLabel;
    m_pClockLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #2d3748;"
        "    background: transparent;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
    );
    m_pClockLabel->setAlignment(Qt::AlignCenter);
    
    // Network status section
    QHBoxLayout *netLayout = new QHBoxLayout;
    netLayout->setSpacing(8);
    netLayout->setContentsMargins(0, 0, 0, 0);
    
    m_pNetPngLabel = new QLabel;
    m_pNetPngLabel->setFixedSize(20, 20);
    m_pNetPngLabel->setScaledContents(true);
    
    m_pNetStatusLabel = new QLabel("Connected");
    m_pNetStatusLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 13px;"
        "    color: #059669;"
        "    font-weight: 600;"
        "    background: transparent;"
        "}"
    );
    
    netLayout->addWidget(m_pNetPngLabel);
    netLayout->addWidget(m_pNetStatusLabel);
    
    // Add to main layout
    mainLayout->addWidget(m_pTitleLabel, 2);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_pClockLabel, 2);
    mainLayout->addStretch(1);
    mainLayout->addLayout(netLayout, 1);
    
    m_timer = new QTimer();
    
    // Clean, professional styling
    q_func()->setStyleSheet(
        "FaceHomeTitleFrm {"
        "    background-color: #ffffff;"
        "    border-bottom: 2px solid #e2e8f0;"
        "}"
    );
}

void FaceHomeTitleFrmPrivate::InitData()
{
    q_func()->setFixedHeight(60);
    q_func()->setObjectName("FaceHomeTitleFrm");
    
    m_timer->start(1000);
    updateClockDisplay();
}

void FaceHomeTitleFrmPrivate::InitConnect()
{
    QObject::connect(m_timer, &QTimer::timeout, [&]{
        updateClockDisplay();
    });
}

void FaceHomeTitleFrmPrivate::updateClockDisplay()
{
    QDateTime displayTime;
    
    if (m_useServerTime && m_serverTime.isValid()) {
        qint64 elapsedMs = m_lastServerTimeUpdate.msecsTo(QDateTime::currentDateTime());
        displayTime = m_serverTime.addMSecs(elapsedMs);
    } else {
        displayTime = QDateTime::currentDateTime();
    }
    
    int index = ReadConfig::GetInstance()->getLanguage_Mode();
    QLocale locale;
    
    if (index == 0)
        locale = QLocale::English;  
    if (index == 1)
        locale = QLocale::English;
    
    QString timeText = locale.toString(displayTime, "yyyy-MM-dd hh:mm:ss");
    m_pClockLabel->setText(timeText);
}

void FaceHomeTitleFrm::setTitleText(const QString &text)
{
    Q_D(FaceHomeTitleFrm);
    d->m_pTitleLabel->setText(text);
}

void FaceHomeTitleFrm::setServerTime(const QDateTime &serverTime)
{
    Q_D(FaceHomeTitleFrm);
    
    d->m_serverTime = serverTime;
    d->m_lastServerTimeUpdate = QDateTime::currentDateTime();
    d->m_useServerTime = true;
    d->updateClockDisplay();
}

void FaceHomeTitleFrm::setLinkState(const bool &state, const int &type)
{
    Q_D(FaceHomeTitleFrm);

    if(state && (type == 1)) {
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_connect.png"));
        d->m_pNetStatusLabel->setText("Ethernet Connected");
        d->m_pNetStatusLabel->setStyleSheet(
            "QLabel { font-size: 13px; color: #059669; font-weight: 600; background: transparent; }"
        );
    }
    else if(state && (type == 2)) {
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/wifi_connect.png"));
        d->m_pNetStatusLabel->setText("WiFi Connected");
        d->m_pNetStatusLabel->setStyleSheet(
            "QLabel { font-size: 13px; color: #059669; font-weight: 600; background: transparent; }"
        );
    }
    else if(!state) {
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_disconnect.png"));
        d->m_pNetStatusLabel->setText("Disconnected");
        d->m_pNetStatusLabel->setStyleSheet(
            "QLabel { font-size: 13px; color: #dc2626; font-weight: 600; background: transparent; }"
        );
    }
    else {
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_connect.png"));
        d->m_pNetStatusLabel->setText("Connected");
        d->m_pNetStatusLabel->setStyleSheet(
            "QLabel { font-size: 13px; color: #059669; font-weight: 600; background: transparent; }"
        );
    }
}

void FaceHomeTitleFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}