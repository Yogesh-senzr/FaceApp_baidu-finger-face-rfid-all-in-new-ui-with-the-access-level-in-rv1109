#include "ImportUserFrm.h"

#ifdef Q_OS_LINUX
#include "USB/UsbObserver.h"
#include <unistd.h>
#endif
#include "OperationTipsFrm.h"
#include "Threads/ParsePersonXlsx.h"
#include "Application/FaceApp.h"

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
    QPushButton *m_pImportButton;//导入
    QProgressBar *m_pImportProgresBar;//导入进度
    QLabel *m_pRecordCountLabel;//总记录数
    QLabel *m_pSucceedRecordCountLabel;//导入成功记录数
    QLabel *m_pFailRecordCountLabel;//导入失败记录数
    QLabel *m_pHintLabel;//升级提示
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
    m_pImportButton = new QPushButton;//导入
    m_pImportProgresBar = new QProgressBar;//导入进度
    m_pRecordCountLabel = new QLabel;//总记录数
    m_pSucceedRecordCountLabel = new QLabel;//导入成功记录数
    m_pFailRecordCountLabel = new QLabel;//导入失败记录数
    m_pHintLabel = new QLabel;

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel(QObject::tr("Step 1:generate excel files using pc tools")), 0, 0);//第一步:使用PC工具批量生成excel文件
    gridLayout->addWidget(new QLabel(QObject::tr("Step 2:copy excel file to U-DISK root director")), 1, 0);//第二步:拷贝excel文件到U盘根目录
    gridLayout->addWidget(new QLabel(QObject::tr("Step 3:click the Import Button")), 2, 0);//第三步:点击下面导入按钮
    gridLayout->setRowMinimumHeight(0, 40);
    gridLayout->setRowMinimumHeight(1, 40);
    gridLayout->setRowMinimumHeight(2, 40);

    QGridLayout *gridLayout1 = new QGridLayout;
    gridLayout1->addWidget(new QLabel(QObject::tr("Total Record Count:")), 0, 0); //总记录数:
    gridLayout1->addWidget(m_pRecordCountLabel, 0, 1);
    gridLayout1->addWidget(new QLabel(QObject::tr("Success Import Count:")));//导入成功记录数:
    gridLayout1->addWidget(m_pSucceedRecordCountLabel, 1, 1);
    gridLayout1->addWidget(new QLabel(QObject::tr("Fail to import Count:")));//导入失败记录数
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

    QHBoxLayout *hLayout3 = new QHBoxLayout;
    hLayout3->addStretch();
    hLayout3->addWidget(m_pImportButton);
    hLayout3->addStretch();

    QHBoxLayout *hLayout4 = new QHBoxLayout;
    hLayout4->addStretch();
    hLayout4->addWidget(m_pHintLabel);
    hLayout4->addStretch();

    QVBoxLayout *maLayout = new QVBoxLayout(q_func());
    maLayout->addSpacing(50);
    maLayout->addLayout(hLayout);
    maLayout->addStretch();
    maLayout->addLayout(hLayout1);
    maLayout->addStretch();
    maLayout->addLayout(hLayout4);
    maLayout->addLayout(hLayout2);
    maLayout->addSpacing(10);
    maLayout->addLayout(hLayout3);
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

    m_pImportButton->setText(QObject::tr("Import"));//导入
    m_pImportButton->setFixedSize(200, 62);
    //设置进度条的范围
    m_pImportProgresBar->setRange(0, 100);
    //设置进度条的当前进度
    m_pImportProgresBar->setValue(0);
    //进度条使用固定式高度
    m_pImportProgresBar->setFixedHeight(17);
    m_pImportProgresBar->setMinimumWidth(350);
}

void ImportUserFrmPrivate::InitConnect()
{
    QObject::connect(m_pImportButton, &QPushButton::clicked, q_func(), &ImportUserFrm::slotImportButtonClicked);
}

void ImportUserFrm::slotImportButtonClicked()
{
    Q_D(ImportUserFrm);
#ifdef Q_OS_LINUX
    //检查U盘是否插入
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState)
    {//判断文件是否存在
        if (access("/udisk/export_person.xlsx", F_OK) == 0)
        {//打开人员表
            d->m_pImportButton->setEnabled(false);
            d->m_pHintLabel->setText(QObject::tr("processing..."));//正在处理文件...
            emit sigParseXlsxPath("/udisk/export_person.xlsx");
        }else
        {
            d->m_pHintLabel->setText(QObject::tr("ImportRecordFail"));//导入失败... import record fail...
        }
    }else
    {
        d->m_pHintLabel->setText(QObject::tr("PlsInsertUDisk"));//请插入U盘...
    }
#else
    //打开人员表
    d->m_pImportButton->setEnabled(false);
    d->m_pHintLabel->setText(QObject::tr("processing..."));//正在处理文件...
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
    }else d->m_pHintLabel->setText(QObject::tr("Import Person Record..."));//正在执行人员导入...

    d->m_pSucceedRecordCountLabel->setText(QString::number(succeedCnt));//导入成功记录数
    d->m_pFailRecordCountLabel->setText(QString::number(failCnt));//导入失败记录数
    //设置进度条的当前进度
    d->m_pImportProgresBar->setValue(index);
}

void ImportUserFrm::setEnter()
{
    Q_D(ImportUserFrm);
    d->m_pHintLabel->clear();
    d->m_pRecordCountLabel->setText("0");//总记录数
    d->m_pSucceedRecordCountLabel->setText("0");//导入成功记录数
    d->m_pFailRecordCountLabel->setText("0");//导入失败记录数
    //设置进度条的范围
    d->m_pImportProgresBar->setRange(1, 100);
    //设置进度条的当前进度
    d->m_pImportProgresBar->setValue(1);

    static bool Init = false;
    if(!Init)
    {
        Init = true;
        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigImportProgressShell, this, &ImportUserFrm::slotProgressShell);
        QObject::connect(this, &ImportUserFrm::sigParseXlsxPath, qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::slotParseXlsxPath);
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void ImportUserFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 
