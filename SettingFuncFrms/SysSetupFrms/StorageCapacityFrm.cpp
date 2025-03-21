#include "StorageCapacityFrm.h"
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStorageInfo>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QDebug>
static inline int queryRowCount(QSqlQuery &query)
{
    int initialPos = query.at();
    // Very strange but for no records .at() returns -2
    int pos = 0;
    if (query.last()) {
        pos = query.at() + 1;
    }
    else {
        pos = 0;
    }
    // Important to restore initial pos
    query.seek(initialPos);
    return pos;
}

class StorageCapacityFrmPrivate
{
    Q_DECLARE_PUBLIC(StorageCapacityFrm)
public:
    StorageCapacityFrmPrivate(StorageCapacityFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pTotalSizeLabel;
    QProgressBar *m_pTotalSizeBar;
    QLabel *m_pTotalCountLabel;//存储记录数
    QProgressBar *m_pTotalCountBar;	
private:
    StorageCapacityFrm *const q_ptr;
};

StorageCapacityFrmPrivate::StorageCapacityFrmPrivate(StorageCapacityFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

StorageCapacityFrm::StorageCapacityFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new StorageCapacityFrmPrivate(this))
{
#ifdef Q_OS_LINUX
    auto storage = QStorageInfo("mnt/user");
#else
    auto storage = QStorageInfo::root();
#endif
    storage.refresh();
    qint64 bytesTotal = storage.bytesTotal();
	qint64 countTotal = bytesTotal *0.9 / (200 * 1024);
	setCountTotal(countTotal);
}

StorageCapacityFrm::~StorageCapacityFrm()
{

}

void StorageCapacityFrmPrivate::InitUI()
{
    m_pTotalSizeLabel = new QLabel;
    m_pTotalSizeBar = new QProgressBar;

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QPushButton);
    //hlayout->addWidget(new QLabel(QObject::tr("剩余空间")));
	hlayout->addWidget(new QLabel(QObject::tr("Used")));//已用空间
    hlayout->addSpacing(50);
    hlayout->addWidget(new QPushButton);
    //hlayout->addWidget(new QLabel(QObject::tr("已用空间")));
	hlayout->addWidget(new QLabel(QObject::tr("Free")));//剩余空间
    hlayout->addStretch();

    m_pTotalCountLabel = new QLabel;
    m_pTotalCountBar = new QProgressBar;
    ((QPushButton *)hlayout->itemAt(0)->widget())->setObjectName("SurplusSpaceButton");
    ((QPushButton *)hlayout->itemAt(3)->widget())->setObjectName("UsedSpaceButton");

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->addSpacing(30);
    vlayout->addWidget(m_pTotalSizeLabel);
    vlayout->addWidget(m_pTotalSizeBar);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_pTotalCountLabel);
    vlayout->addWidget(m_pTotalCountBar);	
    vlayout->addStretch();
}

void StorageCapacityFrmPrivate::InitData()
{
    m_pTotalSizeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pTotalSizeLabel->setText(QObject::tr("TotalDisk:0.0G"));//总空间0.0G
    m_pTotalSizeBar->setFixedHeight(42);
    //m_pTotalSizeBar->setTextVisible(false);
    m_pTotalSizeBar->setAlignment(Qt::AlignCenter);
    m_pTotalSizeBar->setValue(30);
	m_pTotalCountLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pTotalCountLabel->setText(QString("TotalRecords:%1").arg(q_func()->getCountTotal()));//总记录数
	
    m_pTotalCountBar->setFixedHeight(42);
    m_pTotalCountBar->setAlignment(Qt::AlignCenter);
    m_pTotalCountBar->setValue(30);	
}

void StorageCapacityFrmPrivate::InitConnect()
{
}

void StorageCapacityFrm::setEnter()
{
    Q_D(StorageCapacityFrm);


#ifdef Q_OS_LINUX
    auto storage = QStorageInfo("mnt/user");
#else
    auto storage = QStorageInfo::root();
#endif
    storage.refresh();
	qint64 bytesTotal = storage.bytesTotal();

    qint64 bytesFree = storage.bytesFree();

    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    query.prepare("select * from identifyrecord");
    query.exec();
	
	int totalCnt=0;	
	if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        totalCnt = query.size();
    }
    else
    {
        totalCnt = queryRowCount(query);
    }
	

	//按每张相片 200 K 计算
	qint64 countTotal = bytesTotal *0.9 / (200 * 1024);
	qint64 countFree = countTotal-totalCnt;	
    d->m_pTotalSizeLabel->setText(QString(QObject::tr("TotalDisk:%1G")).arg(QString::number(bytesTotal/1024.0/1024.0/1024.0, 'f', 1)));//总空间
    d->m_pTotalSizeBar->setMinimum(0);
    d->m_pTotalSizeBar->setMaximum(bytesTotal/1024); //要除以 1024 ,否则可能 超setMaximum范围, 不能识别出来,
    d->m_pTotalSizeBar->setValue((bytesTotal - bytesFree)/1024);

	d->m_pTotalCountLabel->setText(QString(QObject::tr("TotalRecords:%1")).arg(countTotal));//总记录数
    d->m_pTotalCountBar->setMinimum(0);
    d->m_pTotalCountBar->setMaximum(countTotal);
    d->m_pTotalCountBar->setValue(countTotal - countFree);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void StorageCapacityFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif