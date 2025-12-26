#include "FaceHomeBottomFrm.h"
#include "FaceMainFrm.h"
#include "Config/ReadConfig.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtCore/QDateTime>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

class FaceHomeBottomFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceHomeBottomFrm)
public:
    FaceHomeBottomFrmPrivate(FaceHomeBottomFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();

private:
    // Information panels
    QLabel *m_pTenantLabel;
    QLabel *m_pTenantValue;
    QLabel *m_pSyncLabel;
    QLabel *m_pSyncValue;
    QLabel *m_pStatusLabel;
    QLabel *m_pStatusValue;
    QLabel *m_pNetworkLabel;
    QLabel *m_pNetworkValue;
    QLabel *m_pLocalLabel;
    QLabel *m_pLocalValue;
    QLabel *m_pLastSyncLabel;
    QLabel *m_pLastSyncValue;
    QLabel *m_pHomeIconLabel;
    
private:
    FaceHomeBottomFrm *m_FaceHomeBottomFrm;
private:
    FaceHomeBottomFrm *const q_ptr;
};

FaceHomeBottomFrmPrivate::FaceHomeBottomFrmPrivate(FaceHomeBottomFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceHomeBottomFrm::FaceHomeBottomFrm(QWidget *parent)
    : HomeBottomBaseFrm(parent)
    , d_ptr(new FaceHomeBottomFrmPrivate(this))
{

}

FaceHomeBottomFrm::~FaceHomeBottomFrm()
{

}

void FaceHomeBottomFrmPrivate::InitUI()
{
    // Create main horizontal layout
    QHBoxLayout *mainLayout = new QHBoxLayout(q_func());
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(30);
    
    // Create grid layout for information display
    QGridLayout *infoGrid = new QGridLayout();
    infoGrid->setHorizontalSpacing(35);
    infoGrid->setVerticalSpacing(25);
    
    // Create labels for each information section
    m_pTenantLabel = new QLabel("TENANT");
    m_pTenantValue = new QLabel("Not Available");
    
    m_pSyncLabel = new QLabel("SYNC");
    m_pSyncValue = new QLabel("0/0 Users");
    
    m_pNetworkLabel = new QLabel("NETWORK");
    m_pNetworkValue = new QLabel("Detecting...");
    
    m_pStatusLabel = new QLabel("STATUS");
    m_pStatusValue = new QLabel("Not Started");
    
    m_pLocalLabel = new QLabel("LOCAL");
    m_pLocalValue = new QLabel("0/0 Faces");
    
    m_pLastSyncLabel = new QLabel("LAST SYNC");
    m_pLastSyncValue = new QLabel("Never");
    
    // Style for labels (smaller, uppercase, muted)
    QString labelStyle = 
        "QLabel {"
        "    font-size: 11px;"
        "    font-weight: 600;"
        "    color: #64748b;"
        "    background: transparent;"
        "    margin-bottom: 6px;"
        "}";
        
    // Style for values (larger, darker, more prominent)
    QString valueStyle = 
        "QLabel {"
        "    font-size: 15px;"
        "    font-weight: 500;"
        "    color: #1e293b;"
        "    background: transparent;"
        "    word-wrap: break-word;"
        "}";
    
    // Apply label styles
    m_pTenantLabel->setStyleSheet(labelStyle);
    m_pSyncLabel->setStyleSheet(labelStyle);
    m_pNetworkLabel->setStyleSheet(labelStyle);
    m_pStatusLabel->setStyleSheet(labelStyle);
    m_pLocalLabel->setStyleSheet(labelStyle);
    m_pLastSyncLabel->setStyleSheet(labelStyle);
    
    // Apply value styles
    m_pTenantValue->setStyleSheet(valueStyle);
    m_pSyncValue->setStyleSheet(valueStyle);
    m_pNetworkValue->setStyleSheet(valueStyle);
    m_pStatusValue->setStyleSheet(valueStyle);
    m_pLocalValue->setStyleSheet(valueStyle);
    m_pLastSyncValue->setStyleSheet(valueStyle);
    
    // Set word wrap for tenant value to handle long names
    m_pTenantValue->setWordWrap(true);
    m_pTenantValue->setMaximumWidth(180);
    
    // Create vertical layouts for each info item (label above value)
    QVBoxLayout *tenantLayout = new QVBoxLayout();
    tenantLayout->addWidget(m_pTenantLabel);
    tenantLayout->addWidget(m_pTenantValue);
    tenantLayout->setSpacing(6);
    tenantLayout->setContentsMargins(0, 0, 0, 0);
    
    QVBoxLayout *syncLayout = new QVBoxLayout();
    syncLayout->addWidget(m_pSyncLabel);
    syncLayout->addWidget(m_pSyncValue);
    syncLayout->setSpacing(6);
    syncLayout->setContentsMargins(0, 0, 0, 0);
    
    QVBoxLayout *networkLayout = new QVBoxLayout();
    networkLayout->addWidget(m_pNetworkLabel);
    networkLayout->addWidget(m_pNetworkValue);
    networkLayout->setSpacing(6);
    networkLayout->setContentsMargins(0, 0, 0, 0);
    
    QVBoxLayout *statusLayout = new QVBoxLayout();
    statusLayout->addWidget(m_pStatusLabel);
    statusLayout->addWidget(m_pStatusValue);
    statusLayout->setSpacing(6);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    
    QVBoxLayout *localLayout = new QVBoxLayout();
    localLayout->addWidget(m_pLocalLabel);
    localLayout->addWidget(m_pLocalValue);
    localLayout->setSpacing(6);
    localLayout->setContentsMargins(0, 0, 0, 0);
    
    QVBoxLayout *lastSyncLayout = new QVBoxLayout();
    lastSyncLayout->addWidget(m_pLastSyncLabel);
    lastSyncLayout->addWidget(m_pLastSyncValue);
    lastSyncLayout->setSpacing(6);
    lastSyncLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create container widgets for each info item
    QWidget *tenantWidget = new QWidget();
    tenantWidget->setLayout(tenantLayout);
    
    QWidget *syncWidget = new QWidget();
    syncWidget->setLayout(syncLayout);
    
    QWidget *networkWidget = new QWidget();
    networkWidget->setLayout(networkLayout);
    
    QWidget *statusWidget = new QWidget();
    statusWidget->setLayout(statusLayout);
    
    QWidget *localWidget = new QWidget();
    localWidget->setLayout(localLayout);
    
    QWidget *lastSyncWidget = new QWidget();
    lastSyncWidget->setLayout(lastSyncLayout);
    
    // Add to 3-column grid (2 rows, 3 columns)
    infoGrid->addWidget(tenantWidget, 0, 0, Qt::AlignTop);
    infoGrid->addWidget(syncWidget, 0, 1, Qt::AlignTop);
    infoGrid->addWidget(networkWidget, 0, 2, Qt::AlignTop);
    infoGrid->addWidget(statusWidget, 1, 0, Qt::AlignTop);
    infoGrid->addWidget(localWidget, 1, 1, Qt::AlignTop);
    infoGrid->addWidget(lastSyncWidget, 1, 2, Qt::AlignTop);
    
    // Set column stretches for equal distribution
    infoGrid->setColumnStretch(0, 1);
    infoGrid->setColumnStretch(1, 1);
    infoGrid->setColumnStretch(2, 1);
    
    // Create container widget for grid
    QWidget *infoContainer = new QWidget();
    infoContainer->setLayout(infoGrid);
    
    // Create prominent home button
    m_pHomeIconLabel = new QLabel();
    m_pHomeIconLabel->setPixmap(QPixmap(":/Images/Homeicon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_pHomeIconLabel->setAlignment(Qt::AlignCenter);
    m_pHomeIconLabel->setFixedSize(80, 80);
    m_pHomeIconLabel->setCursor(Qt::PointingHandCursor);
    m_pHomeIconLabel->setStyleSheet(
        "QLabel {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #667eea, stop:1 #764ba2);"
        "    border-radius: 20px;"
        "    border: none;"
        "}"
        "QLabel:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #5a6fd8, stop:1 #6a4190);"
        "}"
        "QLabel:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #4f5bc4, stop:1 #5d3a7e);"
        "}"
    );
    
    // Add to main layout
    mainLayout->addWidget(infoContainer);
    mainLayout->addWidget(m_pHomeIconLabel, 0, Qt::AlignCenter);
    
    // Set stretch factors (info takes most space, button is fixed)
    mainLayout->setStretchFactor(infoContainer, 1);
    mainLayout->setStretchFactor(m_pHomeIconLabel, 0);
    
    // Modern panel styling
    q_func()->setStyleSheet(
        "FaceHomeBottomFrm {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1);"
        "}"
    );
}

void FaceHomeBottomFrm::setNetInfo(const QString &address, const QString &make)
{
    Q_D(FaceHomeBottomFrm);
    
    bool showIP = ReadConfig::GetInstance()->getHomeDisplay_DisplayIP();
    
    if (showIP && d->m_pNetworkValue) {
        if (address.isEmpty()) {
            d->m_pNetworkValue->setText("Not Available");
        } else {
            d->m_pNetworkValue->setText(address);
        }
        d->m_pNetworkLabel->show();
        d->m_pNetworkValue->show();
    } else {
        d->m_pNetworkLabel->hide();
        d->m_pNetworkValue->hide();
    }
}

void FaceHomeBottomFrmPrivate::InitData()
{
    // Set initial values
    m_pTenantValue->setText("Not Available");
    m_pSyncValue->setText("0/0 Users");
    m_pStatusValue->setText("Not Started");
    m_pNetworkValue->setText("Detecting...");
    m_pLocalValue->setText("0/0 Faces");
    m_pLastSyncValue->setText("Never");

    // Increase height significantly for better spacing
    q_func()->setFixedHeight(220);
}

void FaceHomeBottomFrmPrivate::InitConnect()
{
    // No connections needed for now
}

void FaceHomeBottomFrm::setTenantName(const QString &tenantName)
{
    Q_D(FaceHomeBottomFrm);
    if (tenantName.isEmpty()) {
        d->m_pTenantValue->setText("Not Available");
    } else {
        d->m_pTenantValue->setText(tenantName);
    }
}

void FaceHomeBottomFrm::setSyncUserCount(int currentCount, int totalCount)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pSyncValue->setText(QString("%1/%2 Users").arg(currentCount).arg(totalCount));
}

void FaceHomeBottomFrm::setSyncStatus(const QString &status)
{
    Q_D(FaceHomeBottomFrm);
    QString statusText;
    QString statusStyle = "QLabel { font-size: 15px; font-weight: 500; background: transparent; word-wrap: break-word; ";
    
    if (status.toLower() == "completed") {
        statusText = "✅ Completed";
        statusStyle += "color: #059669; }";
    } else if (status.toLower() == "running" || status.toLower() == "syncing") {
        statusText = "⏳ Running";
        statusStyle += "color: #d97706; }";
    } else if (status.toLower() == "failed") {
        statusText = "❌ Failed";
        statusStyle += "color: #dc2626; }";
    } else {
        statusText = status;
        statusStyle += "color: #1e293b; }";
    }
    
    d->m_pStatusValue->setStyleSheet(statusStyle);
    d->m_pStatusValue->setText(statusText);
}

void FaceHomeBottomFrm::setLastSyncTime(const QString &time)
{
    Q_D(FaceHomeBottomFrm);
    if (time.isEmpty()) {
        d->m_pLastSyncValue->setText("Never");
    } else {
        d->m_pLastSyncValue->setText(time);
    }
}

void FaceHomeBottomFrm::setLocalFaceCount(int localCount, int totalCount)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pLocalValue->setText(QString("%1/%2 Faces").arg(localCount).arg(totalCount));
}

void FaceHomeBottomFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

void FaceHomeBottomFrm::mousePressEvent(QMouseEvent *event)
{
    Q_D(FaceHomeBottomFrm);
    
    // Check if the click is on the home icon
    if (d->m_pHomeIconLabel && d->m_pHomeIconLabel->geometry().contains(event->pos())) {
        qDebug() << "Home icon clicked - triggering settings access";
        
        // Visual feedback - pressed state
        d->m_pHomeIconLabel->setStyleSheet(
            "QLabel {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
            "                stop:0 #4f5bc4, stop:1 #5d3a7e);"
            "    border-radius: 20px;"
            "    border: none;"
            "}"
        );
        
        // Reset after brief moment
        QTimer::singleShot(150, [d]() {
            d->m_pHomeIconLabel->setStyleSheet(
                "QLabel {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                "                stop:0 #667eea, stop:1 #764ba2);"
                "    border-radius: 20px;"
                "    border: none;"
                "}"
                "QLabel:hover {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                "                stop:0 #5a6fd8, stop:1 #6a4190);"
                "}"
                "QLabel:pressed {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                "                stop:0 #4f5bc4, stop:1 #5d3a7e);"
                "}"
            );
        });
        
        // Trigger settings
        QWidget *mainWindow = this;
        while (mainWindow->parent() && qobject_cast<QWidget*>(mainWindow->parent())) {
            mainWindow = qobject_cast<QWidget*>(mainWindow->parent());
        }
        
        FaceMainFrm *faceMainFrm = qobject_cast<FaceMainFrm*>(mainWindow);
        if (faceMainFrm) {
            qDebug() << "Calling triggerSettings on FaceMainFrm";
            faceMainFrm->triggerSettings();
        }
    }
    
    QWidget::mousePressEvent(event);
}