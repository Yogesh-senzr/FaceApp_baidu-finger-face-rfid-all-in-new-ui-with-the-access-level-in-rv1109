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

#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlDriver>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QTableView>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QStackedWidget>

#define PAGE (20)
#define DateTime QDateTime::currentDateTime().toString("yyyy-MM-dd")

class UserViewFrmPrivate
{
    Q_DECLARE_PUBLIC(UserViewFrm)
public:
    UserViewFrmPrivate(UserViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    //void AddProjectInfo(const QString &name, const QString &sex, const QString &iccardnum, const QString &idcardnum, const QString &creadtime);
    void AddProjectInfo(const QString &name, const QString &sex, const QString &iccardnum, const QString &idcardnum, \
        const QString &creadtime, const QString &personid,const QString &uuid);
    void SelectData(const QString &sql);
private:
    void ClearTableData();
private:
    QWidget *newButton();
private:
    //界面框架主页
    FaceHomeFrm *m_pFaceHomeFrm;
private:
    QStackedWidget *m_pStackedWidget;     
private:
    QLineEdit *m_pInputDataEdit;//信息录入框
    QPushButton *m_pQueryButton;//查询
    QPushButton *m_pExportButton;//导出
	QProgressDialog *m_pProgressDialog; //进度条	
private:
    QTableView *m_pTableView;
    QStandardItemModel *m_pTableViewModel;
private:
    PageNavigator *m_pPageNavigator;
private:
    UserViewFrm *const q_ptr;
};

UserViewFrmPrivate::UserViewFrmPrivate(UserViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
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
    m_pInputDataEdit = new QLineEdit;//信息录入框
    m_pQueryButton = new QPushButton;//查询
    m_pExportButton = new QPushButton;//导出
    m_pPageNavigator = new PageNavigator;
    m_pTableView = new QTableView;
    m_pTableViewModel = new QStandardItemModel;
    m_pInputDataEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(20, 0, 20, 10);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(10);
    layout->addWidget(m_pInputDataEdit);
    layout->addWidget(m_pQueryButton);
    layout->addWidget(m_pExportButton);

    vlayout->addLayout(layout);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_pTableView);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_pPageNavigator);
}

void UserViewFrmPrivate::InitData()
{
    QStringList tableHeader;
    //tableHeader << QObject::tr("姓名") << QObject::tr("性别") << QObject::tr("卡号") << QObject::tr("身份证") << QObject::tr("添加时间") << QObject::tr("操作");
    //tableHeader << QObject::tr("Name") << QObject::tr("Sex") << QObject::tr("CardNo") << QObject::tr("IdCard") << QObject::tr("AppendTime") << QObject::tr("Action");
    tableHeader << QObject::tr("Name") << QObject::tr("Sex") << QObject::tr("CardNo") << QObject::tr("IdCard") << QObject::tr("AppendTime") << QObject::tr("PersionId") << QObject::tr("Uuid")<< QObject::tr("Action");
    m_pTableViewModel->setHorizontalHeaderLabels(tableHeader);
    m_pTableViewModel->setColumnCount(tableHeader.count());

    m_pTableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);    
    m_pTableView->setEditTriggers(QTableView::NoEditTriggers);
    m_pTableView->setItemDelegate(new wholeDelegate);
    //m_pTableView->setItemDelegateForColumn(tableHeader.count()- 1, new ButtonDelegate);
    m_pTableView->setModel(m_pTableViewModel);

    m_pQueryButton->setText(QObject::tr("Search"));//查询
    m_pExportButton->setText(QObject::tr("Export"));//导出

    m_pInputDataEdit->setPlaceholderText(QObject::tr("Name/CardNo/IdCard"));//姓名/卡号/身份证
    m_pQueryButton->setFixedSize(100, 52);
    m_pExportButton->setFixedSize(100, 52);
}

void UserViewFrmPrivate::InitConnect()
{
    QObject::connect(m_pQueryButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotQueryButton);
    QObject::connect(m_pPageNavigator, &PageNavigator::currentPageChanged, q_func(), &UserViewFrm::slotCurrentPageChanged);
    QObject::connect(m_pExportButton, &QPushButton::clicked, q_func(), &UserViewFrm::slotExportButton);
    QObject::connect(m_pTableView, SIGNAL(doubleClicked(const QModelIndex &)), q_func(), SLOT(slotRowDoubleClicked(const QModelIndex &)));    
}

void UserViewFrmPrivate::SelectData(const QString &sql)
{
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(sql);
    query.exec();
    while (query.next())
    {//取出对应数据
        //this->AddProjectInfo(query.value("name").toString(), query.value("sex").toString(), query.value("iccardnum").toString(), query.value("idcardnum").toString(), query.value("createtime").toString());
        this->AddProjectInfo(query.value("name").toString(), query.value("sex").toString(), query.value("iccardnum").toString(), \
            query.value("idcardnum").toString(), query.value("createtime").toString(), query.value("personid").toString(),\
            query.value("uuid").toString() );
    }
}

void UserViewFrmPrivate::ClearTableData()
{
    this->m_pTableViewModel->removeRows(0, this->m_pTableViewModel->rowCount());
}

QWidget *UserViewFrmPrivate::newButton()
{
    auto del = new QPushButton(QObject::tr("Delete"));//删除
    QObject::connect(del, &QPushButton::clicked, q_func(), &UserViewFrm::slotDeleteButton);
    return del;
}

void UserViewFrmPrivate::AddProjectInfo(const QString &name, const QString &sex, const QString &iccardnum, const QString &idcardnum,
   const QString &creadtime, const QString &personid,const QString &uuid)
{
    QList<QStandardItem*> list;
    //list << new QStandardItem(name) << new QStandardItem(sex) << new QStandardItem(iccardnum) << new QStandardItem(idcardnum) << new QStandardItem(creadtime) <<new QStandardItem("");
    list << new QStandardItem(name) << new QStandardItem(sex) << new QStandardItem(iccardnum) << new QStandardItem(idcardnum) 
        << new QStandardItem(creadtime) << new QStandardItem(personid)<< new QStandardItem(uuid)<<new QStandardItem("");
    m_pTableViewModel->appendRow(list);
    int nRowCount = m_pTableViewModel->rowCount() - 1;
    m_pTableView->setRowHeight(nRowCount, 52);
    m_pTableView->setColumnWidth(0, 120);//name
    m_pTableView->setColumnWidth(1, 120);//sex
    m_pTableView->setColumnWidth(2, 240);   //iccardnum 
    m_pTableView->setColumnWidth(3, 240);//idcardnum
    m_pTableView->setColumnWidth(4, 240);//createtime
    m_pTableView->setColumnWidth(5, 120);  //personid     
    m_pTableView->setIndexWidget(m_pTableViewModel->index(nRowCount, m_pTableViewModel->columnCount() - 1), this->newButton());
}

static inline QString PackLikeselect(const QString &text)
{
    QString strSql;
    strSql = QString("select * from person where name like '%%1").arg(text);
    strSql.append("%' or ");
    strSql.append(QString("idcardnum like '%%1").arg(text));
    strSql.append("%' or ");
    strSql.append(QString("iccardnum like '%%1").arg(text));
    strSql.append("%'");
    return strSql;
}

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

static inline int SelectDataCount(const QString &sql)
{
    int count = 0;
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));

    query.exec(sql);
    if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        count = query.size();
    }
    else
    {
        count = queryRowCount(query);
    }

    int page = 0;
    if (count <= PAGE)
    {
        page = 1;
    }
    else
    {
        page = count / PAGE;
        if ((count % PAGE) != 0)++page;
    }
    return page;
}


                  
void UserViewFrm::slotRowDoubleClicked(const QModelIndex &)
{
    Q_D(UserViewFrm);    
    QModelIndex index = d->m_pTableView->currentIndex();
 //                         0    3                  2             1                 5             6
//modifyRecord(QString aName,QString aIDCard,QString aCardNo,QString asex,QString apersonid,QString auuid)
//210 name= "77" ,aIDCard= "男" ,aCardNo= "uuC666666" ,asex= "男" ,apersonid= "2" ,auuid= "{84d2b98b-cd53-4813-8a54-3edcd109e6ba}"
    AddUserFrm::GetInstance()->modifyRecord(d->m_pTableViewModel->item(index.row(),0)->text(), d->m_pTableViewModel->item(index.row(),3)->text(), \
          d->m_pTableViewModel->item(index.row(),2)->text(), d->m_pTableViewModel->item(index.row(),1)->text(), \
          d->m_pTableViewModel->item(index.row(),5)->text(), d->m_pTableViewModel->item(index.row(),6)->text());          

    emit sigShowFrm(QObject::tr("ModifyPerson"));  //AddPerson  
    
    //this->slotQueryButton();
}
void UserViewFrm::setEnter()
{
    Q_D(UserViewFrm);
    d->m_pQueryButton->setFocus();
    static bool Init = false;
    if(!Init)
    {
        Init = true;
        QObject::connect(qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::sigExportProgressShell, this, &UserViewFrm::slotExportProgressShell);
        QObject::connect(this, &UserViewFrm::sigExportPersons, qXLApp->GetParsePersonXlsx(), &ParsePersonXlsx::slotExportPersons);
    }
    this->slotQueryButton();
}

void UserViewFrm::slotCurrentPageChanged(const int page)
{
    Q_D(UserViewFrm);
    Q_UNUSED(page);
    d->m_pQueryButton->setFocus();
    d->ClearTableData();
    QString text = d->m_pInputDataEdit->text();
    if (text.isEmpty())
    {
        d->SelectData((QString("SELECT *FROM person LIMIT %1,%2").arg((page - 1) *PAGE).arg(PAGE)));
    }
    else
    {
        text = ::PackLikeselect(text);
        d->SelectData((text + QString(" order by personid desc LIMIT %1,%2").arg((page - 1) *PAGE).arg(PAGE)));
    }
}

void UserViewFrm::slotQueryButton()
{
    Q_D(UserViewFrm);
    d->ClearTableData();

    QString text = d->m_pInputDataEdit->text();
    QString selSQL, selSQLCnt;    
    selSQLCnt = ::PackLikeselect(text);
    d->m_pPageNavigator->setMaxPage(SelectDataCount(selSQLCnt));
    selSQL = selSQLCnt + QString(" order by personid desc LIMIT 0,%1").arg(PAGE);

    d->SelectData(selSQL);
}


void UserViewFrm::slotDeleteButton()
{
    Q_D(UserViewFrm);
    //"确实要删除此项数据吗?"
    if (QMessageBox::question(this, QObject::tr("Tips"), QObject::tr("AreUMakeSureDelete"),
        QMessageBox::Yes|QMessageBox::No)== QMessageBox::No)
    {
        return ;
    }


    QPushButton *pItemWidget = (QPushButton *)sender();
    for(int row = 0; d->m_pTableViewModel->rowCount(); row++)
    {
        auto w = d->m_pTableView->indexWidget(d->m_pTableViewModel->index(row, d->m_pTableViewModel->columnCount() - 1));
        if(pItemWidget == w)
        {
            RegisteredFacesDB::GetInstance()->DelPersonDBInfo(d->m_pTableViewModel->index(row, 0).data().toString(),  d->m_pTableViewModel->index(row, 4).data().toString());
            break;
        }
    }
    this->slotQueryButton();
}

void UserViewFrm::slotExportButton()
{
    Q_D(UserViewFrm);
#ifdef Q_OS_LINUX
    //检查U盘是否插入
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState)
    {
        d->m_pExportButton->setEnabled(false);
        d->m_pExportButton->setText(QObject::tr("Export..."));//导出中...
		d->m_pProgressDialog = new QProgressDialog(this);
		QFont font("YSong18030", 12);
		d->m_pProgressDialog->setFont(font);
		d->m_pProgressDialog->setWindowModality(Qt::WindowModal);
		d->m_pProgressDialog->setAttribute(Qt::WA_DeleteOnClose);		
		d->m_pProgressDialog->setMinimumDuration(5);
		d->m_pProgressDialog->setWindowTitle(QObject::tr("PlsWaiting"));//请稍等
		d->m_pProgressDialog->setLabelText(QObject::tr("Export..."));//导出中...
		d->m_pProgressDialog->setCancelButtonText(NULL);		
        emit sigExportPersons();
    }else
    {
        OperationTipsFrm dlg(this);
       // dlg.setMessageBox(QObject::tr("温馨提示"), QObject::tr("请插入U盘"), QObject::tr("确定"), QString(), 1);
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("PlsInsertUDisk"), QObject::tr("Ok"), QString(), 1);	   
    }
#else
    d->m_pExportButton->setEnabled(false);
    d->m_pExportButton->setText(QObject::tr("Export..."));//导出中...
    emit sigExportPersons();
#endif
}

void UserViewFrm::slotExportProgressShell(const bool dealstate, const bool savestate, const int total, const int dealcnt)
{
    Q_D(UserViewFrm);
    Q_UNUSED(savestate);
    Q_UNUSED(total);
    Q_UNUSED(dealcnt);
    if(dealstate)
    {
		int icnt =0;
		
		d->m_pProgressDialog->setModal(true); 			
		d->m_pProgressDialog->show();		
		if (total<20)
		{			
			icnt =100 / total;
		
			d->m_pProgressDialog->setRange(0, 100);	
			d->m_pProgressDialog->setValue(dealcnt*icnt);
			
			//msleep(500);		
			QTime dieTime = QTime::currentTime().addMSecs(500);//500*1000, 2

			while( QTime::currentTime() < dieTime )
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
				
		} else 
		{
			d->m_pProgressDialog->setRange(0, total);	
			d->m_pProgressDialog->setValue(dealcnt);						
		}
				
		if (total == dealcnt)
		{						
			d->m_pExportButton->setEnabled(true);
			d->m_pExportButton->setText(QObject::tr("Export"));//导出			
			d->m_pProgressDialog->close();
		}
    }
}
void UserViewFrm::setModifyFlag( int value)
{  
    Q_D(UserViewFrm);   
    this->slotQueryButton();     
    mModifyFlag = value;

    d->ClearTableData();

    QString text = d->m_pInputDataEdit->text();
    QString selSQL, selSQLCnt;
    if (text.isEmpty())
    {
        selSQLCnt ="SELECT *FROM person ";
    }
    else
    {
        selSQLCnt = ::PackLikeselect(text);
    }
    d->m_pPageNavigator->setMaxPage(SelectDataCount(selSQLCnt));
    selSQL = selSQLCnt + QString(" order by personid desc LIMIT 0,%1").arg(PAGE);
    d->SelectData(selSQL);    

    //this->showNormal();
    //this->show();
    this->update();
    this->repaint();    
    this->resize(this->size());
    this->adjustSize();
    QApplication::processEvents(); 
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void UserViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif