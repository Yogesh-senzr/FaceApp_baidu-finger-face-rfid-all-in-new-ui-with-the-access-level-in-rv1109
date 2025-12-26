#include "ImportUserFrm.h"

#ifdef Q_OS_LINUX
#include "USB/UsbObserver.h"
#include <unistd.h>
#endif
#include "OperationTipsFrm.h"
#include "Threads/ParsePersonXlsx.h"
#include "Application/FaceApp.h"
#include "HttpServer/ConnHttpServerThread.h"
#include "DB/RegisteredFacesDB.h"

#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QEventLoop>

class ImportUserFrmPrivate
{
    Q_DECLARE_PUBLIC(ImportUserFrm)
public:
    ImportUserFrmPrivate(ImportUserFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pImportButton;
    QPushButton *m_pAddFaceButton; // Add face data button
    QProgressBar *m_pImportProgresBar;
    QLabel *m_pRecordCountLabel;
    QLabel *m_pSucceedRecordCountLabel;
    QLabel *m_pFailRecordCountLabel;
    QLabel *m_pHintLabel;
private:
    ImportUserFrm *const q_ptr;
};

ImportUserFrmPrivate::ImportUserFrmPrivate(ImportUserFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ImportUserFrm::ImportUserFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new ImportUserFrmPrivate(this))
{

}

ImportUserFrm::~ImportUserFrm()
{

}

void ImportUserFrmPrivate::InitUI()
{
    m_pImportButton = new QPushButton;
    m_pImportProgresBar = new QProgressBar;
    m_pRecordCountLabel = new QLabel;
    m_pSucceedRecordCountLabel = new QLabel;
    m_pFailRecordCountLabel = new QLabel;
    m_pHintLabel = new QLabel;

    m_pAddFaceButton = new QPushButton;
    m_pAddFaceButton->setText(QObject::tr("Import FaceID")); 
    m_pAddFaceButton->setFixedSize(200, 62);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel(QObject::tr("Step 1:generate excel files using pc tools")), 0, 0);
    gridLayout->addWidget(new QLabel(QObject::tr("Step 2:copy excel file to U-DISK root director")), 1, 0);
    gridLayout->addWidget(new QLabel(QObject::tr("Step 3:click the Import FaceID")), 2, 0);
    gridLayout->setRowMinimumHeight(0, 40);
    gridLayout->setRowMinimumHeight(1, 40);
    gridLayout->setRowMinimumHeight(2, 40);

    QGridLayout *gridLayout1 = new QGridLayout;
    gridLayout1->addWidget(new QLabel(QObject::tr("Total Record Count:")), 0, 0); 
    gridLayout1->addWidget(m_pRecordCountLabel, 0, 1);
    gridLayout1->addWidget(new QLabel(QObject::tr("Success Import Count:")));
    gridLayout1->addWidget(m_pSucceedRecordCountLabel, 1, 1);
    gridLayout1->addWidget(new QLabel(QObject::tr("Fail to import Count:")));
    gridLayout1->addWidget(m_pFailRecordCountLabel, 2, 1);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addLayout(gridLayout);
    hLayout->addStretch();

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addLayout(gridLayout1);
    hLayout1->addStretch();

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addStretch();
    hLayout2->addWidget(m_pImportProgresBar);
    hLayout2->addStretch();

    /*QHBoxLayout *hLayout3 = new QHBoxLayout;
    hLayout3->addStretch();
    hLayout3->addWidget(m_pImportButton);
    hLayout3->addStretch();*/

    QHBoxLayout *hLayout4 = new QHBoxLayout;
    hLayout4->addStretch();
    hLayout4->addWidget(m_pHintLabel);
    hLayout4->addStretch();

    QHBoxLayout *hLayout5 = new QHBoxLayout;
    hLayout5->addStretch();
    hLayout5->addWidget(m_pAddFaceButton);
    hLayout5->addStretch();

    QVBoxLayout *maLayout = new QVBoxLayout(q_func());
    maLayout->addSpacing(50);
    maLayout->addLayout(hLayout);
    maLayout->addStretch();
    maLayout->addLayout(hLayout1);
    maLayout->addStretch();
    maLayout->addLayout(hLayout4);
    maLayout->addLayout(hLayout2);
    maLayout->addSpacing(10);
    //maLayout->addLayout(hLayout3);
   // maLayout->addSpacing(10);
    maLayout->addLayout(hLayout5);
    maLayout->addStretch();
}

void ImportUserFrmPrivate::InitData()
{
    QStringList qss;
    qss.append(QString("QProgressBar{font:5pt;height:6px;color:white;background:grey;border-radius:6px;text-align:center;border:0px solid grey;}"));
    qss.append(QString("QProgressBar::chunk { background-color: #05B8CC; width: 1px; margin: 0px; }"));
    qss.append(QString("QLabel{color:black;}"));
    q_func()->setStyleSheet(qss.join(""));

    m_pRecordCountLabel->setText("0");
    m_pSucceedRecordCountLabel->setText("0");
    m_pFailRecordCountLabel->setText("0");

    m_pImportButton->setText(QObject::tr("Import"));
    m_pImportButton->setFixedSize(200, 62);
    m_pImportProgresBar->setRange(0, 100);
    m_pImportProgresBar->setValue(0);
    m_pImportProgresBar->setFixedHeight(17);
    m_pImportProgresBar->setMinimumWidth(350);
}

void ImportUserFrmPrivate::InitConnect()
{
    QObject::connect(m_pImportButton, &QPushButton::clicked, q_func(), &ImportUserFrm::slotImportButtonClicked);
    QObject::connect(m_pAddFaceButton, &QPushButton::clicked, q_func(), &ImportUserFrm::slotAddFaceButtonClicked);

}

void ImportUserFrm::slotAddFaceButtonClicked()
{
    Q_D(ImportUserFrm);
#ifdef Q_OS_LINUX
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState) {
        if (access("/udisk/face_images.xlsx", F_OK) == 0) {
            d->m_pAddFaceButton->setEnabled(false);
            d->m_pHintLabel->setText(QObject::tr("Adding face data..."));
            emit sigAddFaceFromXlsx("/udisk/face_images.xlsx");
        } else {
            d->m_pHintLabel->setText(QObject::tr("Face images file not found")); 
        }
    } else {
        d->m_pHintLabel->setText(QObject::tr("Please insert U-disk")); 
    }
#else
    d->m_pAddFaceButton->setEnabled(false);
    d->m_pHintLabel->setText(QObject::tr("Adding face data...")); 
    emit sigAddFaceFromXlsx("C:\\Users\\63279\\Desktop\\人脸产品\\测试资源\\测试导入2万人脸\\face_images.xlsx");
#endif
}

void ImportUserFrm::slotAddFaceProgressShell(bool isFinished, int total, int current, int successCount, int failCount)
{
    Q_D(ImportUserFrm);

    if(d->m_pImportProgresBar->maximum() != total) {
        d->m_pRecordCountLabel->setText(QString::number(total));
        d->m_pImportProgresBar->setRange(1, total);
    }

    if(isFinished) {
        d->m_pHintLabel->setText(QObject::tr("Face data addition completed")); 
        d->m_pAddFaceButton->setEnabled(true);
    } else {
        d->m_pHintLabel->setText(QObject::tr("Adding face data...")); 
    }

    d->m_pSucceedRecordCountLabel->setText(QString::number(successCount));
    d->m_pFailRecordCountLabel->setText(QString::number(failCount));
    d->m_pImportProgresBar->setValue(current);
}


void ImportUserFrm::slotImportButtonClicked()
{
    Q_D(ImportUserFrm);
#ifdef Q_OS_LINUX
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState)
    {
        if (access("/udisk/export_person.xlsx", F_OK) == 0)
        {
            d->m_pImportButton->setEnabled(false);
            d->m_pHintLabel->setText(QObject::tr("processing..."));
            emit sigParseXlsxPath("/udisk/export_person.xlsx");
        }else
        {
            d->m_pHintLabel->setText(QObject::tr("ImportRecordFail"));  
        }
    }else
    {
        d->m_pHintLabel->setText(QObject::tr("PlsInsertUDisk"));
    }
#else
    d->m_pImportButton->setEnabled(false);
    d->m_pHintLabel->setText(QObject::tr("processing..."));
    emit sigParseXlsxPath("C:\\Users\\63279\\Desktop\\人脸产品\\测试资源\\测试导入2万人脸\\export_person.xlsx");
#endif
}

void ImportUserFrm::slotProgressShell(const bool state, const int total, const int index, const int succeedCnt, const int failCnt)
{
    Q_D(ImportUserFrm);

    if(d->m_pImportProgresBar->maximum() != total)
    {
        d->m_pRecordCountLabel->setText(QString::number(total));
        d->m_pImportProgresBar->setRange(1, total);
    }

    if(state)
    {
        d->m_pHintLabel->clear();
        d->m_pImportButton->setEnabled(true);
    }else d->m_pHintLabel->setText(QObject::tr("Import Person Record..."));

    d->m_pSucceedRecordCountLabel->setText(QString::number(succeedCnt));
    d->m_pFailRecordCountLabel->setText(QString::number(failCnt));
    d->m_pImportProgresBar->setValue(index);
}

void ImportUserFrm::slotFaceDataUpdated(const QString &uuid)
{
    // Get the updated user data
    QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(uuid);
    if (persons.size() > 0) {
        PERSONS_t person = persons[0];
        
        // Send to server if face feature exists
        if (!person.feature.isEmpty()) {
            ConnHttpServerThread::GetInstance()->sendUserToServer(
                person.uuid, person.name, person.idcard, person.iccard, 
                person.sex, person.department, person.timeOfAccess, person.feature);
        }
    }
    
    // Trigger UI refresh
    emit sigUpdateUserList();
}

void ImportUserFrm::setEnter()
{
    Q_D(ImportUserFrm);
    d->m_pHintLabel->clear();
    d->m_pRecordCountLabel->setText("0");
    d->m_pSucceedRecordCountLabel->setText("0");
    d->m_pFailRecordCountLabel->setText("0");
    
    d->m_pImportProgresBar->setRange(1, 100);
    
    d->m_pImportProgresBar->setValue(1);

    static bool Init = false;
    if(!Init)
    {
        Init = true;
        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigImportProgressShell, this, &ImportUserFrm::slotProgressShell);
        QObject::connect(this, &ImportUserFrm::sigParseXlsxPath, qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::slotParseXlsxPath);

        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigAddFaceProgressShell, 
                 this, &ImportUserFrm::slotAddFaceProgressShell);
        QObject::connect(this, &ImportUserFrm::sigAddFaceFromXlsx, 
                 qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::slotAddFaceDataFromXlsx);
        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigFaceDataUpdated,
                 this, &ImportUserFrm::slotFaceDataUpdated);         
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void ImportUserFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 
