#include "ViewAccessRecordsFrm.h"
#include "Delegate/wholedelegate.h"
#include "pagenavigator.h"
#include "FacePngFrm.h"

#include "Threads/RecordsExport.h"
#include "OperationTipsFrm.h"
#include "Application/FaceApp.h"

#ifdef Q_OS_LINUX
#include "USB/UsbObserver.h"
#endif

#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlDriver>
#include <QTabBar>
#include <QTableView>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDateEdit>
#include <QCalendarWidget>

#define PAGE (20)
#define DateTime QDateTime::currentDateTime().toString("yyyy-MM-dd")

class ViewAccessRecordsFrmPrivate
{
    Q_DECLARE_PUBLIC(ViewAccessRecordsFrm)
public:
    ViewAccessRecordsFrmPrivate(ViewAccessRecordsFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void AddProjectInfo(const QString &name, const QString &, const QString &, const QString &, const QString &);
    void SelectData(const QString &sql);
private:
    void ClearTableData();
private:
    //QLineEdit *m_pStartTimerEdit;//起始时间录入框
    //QLineEdit *m_pEndTimerEdit;//结束时间录入框

    QDateEdit *m_pStartTimerEdit;//起始时间录入框
    QDateEdit *m_pEndTimerEdit;//结束时间录入框

    QCalendarWidget *m_pStartCalendarWidget; 
    QCalendarWidget *m_pEndCalendarWidget; 

    QPushButton *m_pQueryButton;//查询
    QPushButton *m_pExportButton;//导出
    QPushButton *m_pCleanButton;//清除
	QProgressDialog *m_pProgressDialog; //进度条
private:
    PageNavigator *m_pPageNavigator;
private:
    QTableView *m_pTableView;
    QStandardItemModel *m_pTableViewModel;
private:
    ViewAccessRecordsFrm *const q_ptr;
};

ViewAccessRecordsFrmPrivate::ViewAccessRecordsFrmPrivate(ViewAccessRecordsFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ViewAccessRecordsFrm::ViewAccessRecordsFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new ViewAccessRecordsFrmPrivate(this))
{
}

ViewAccessRecordsFrm::~ViewAccessRecordsFrm()
{

}

void ViewAccessRecordsFrmPrivate::InitUI()
{
    //m_pStartTimerEdit = new QLineEdit;//起始时间录入框
    //m_pEndTimerEdit = new QLineEdit;//结束时间录入框
    m_pStartTimerEdit = new QDateEdit;//起始时间录入框
    m_pEndTimerEdit = new QDateEdit;//结束时间录入框    

    m_pStartCalendarWidget = new QCalendarWidget; 
    m_pEndCalendarWidget = new QCalendarWidget; 
    
    m_pQueryButton = new QPushButton;//查询
    m_pExportButton = new QPushButton;//导出
    m_pCleanButton = new QPushButton;//清除

    m_pStartTimerEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pEndTimerEdit->setContextMenuPolicy(Qt::NoContextMenu);    

    m_pTableView = new QTableView;
    m_pTableViewModel = new QStandardItemModel;
    m_pPageNavigator = new PageNavigator;

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(20, 0, 20, 10);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(10);
    layout->addWidget(m_pStartTimerEdit);
    layout->addWidget(m_pEndTimerEdit);
    layout->addWidget(m_pQueryButton);
    layout->addWidget(m_pExportButton);
    layout->addWidget(m_pCleanButton);

    vlayout->addLayout(layout);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_pTableView);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_pPageNavigator);
}

void ViewAccessRecordsFrmPrivate::InitData()
{
    QStringList tableHeader;
    //tableHeader << QObject::tr("姓名") << QObject::tr("性别") << QObject::tr("温度") << QObject::tr("通行时间") << QObject::tr("人脸");
    tableHeader << QObject::tr("Name") << QObject::tr("Sex") << QObject::tr("Temperature") << QObject::tr("PassTime") << QObject::tr("FaceImage");
    m_pTableViewModel->setHorizontalHeaderLabels(tableHeader);
    m_pTableViewModel->setColumnCount(tableHeader.count());

    m_pTableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pTableView->setEditTriggers(QTableView::NoEditTriggers);
    m_pTableView->setItemDelegate(new wholeDelegate);
    //m_pTableView->setItemDelegateForColumn(tableHeader.count()- 1, new ButtonDelegate);
    m_pTableView->setModel(m_pTableViewModel);
    
    m_pTableView->setContextMenuPolicy(Qt::NoContextMenu);
    m_pQueryButton->setText(QObject::tr("Search"));//查询
    m_pExportButton->setText(QObject::tr("Export"));//导出
    m_pCleanButton->setText(QObject::tr("Clean"));//清除

QString buttonStyle = QString(
    "QPushButton {"
    "    font-family: '黑体';"
    "    font-size: 6pt;"  // Even smaller to ensure we see a change
    "    font-weight: normal;"
    "    padding: 2px;"
    "    min-height: 52px;"
    "    max-height: 52px;"
    "    width: 60px;"
    "}"
);

m_pQueryButton->setStyleSheet(buttonStyle);
m_pExportButton->setStyleSheet(buttonStyle);
m_pCleanButton->setStyleSheet(buttonStyle);
    


   // m_pStartTimerEdit->setAlignment(Qt::AlignCenter);
    m_pStartTimerEdit->setCalendarPopup(true);
    //m_pStartTimerEdit->setDisplayFormat("yyyy-MM-dd");
    m_pStartTimerEdit->setDisplayFormat("yyyy/MM/dd");
    //m_pStartTimerEdit->setDate(QDate::currentDate());
    m_pStartTimerEdit->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(),1));
    QFont font("黑体", 8);//14
    m_pStartTimerEdit->setFont(font);
    m_pStartTimerEdit->setStyleSheet(QString("QDateEdit{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
                                        "stop:0 rgba(250, 250, 250, 255), "
                                        "stop:0.5 rgba(240, 240, 240, 255), "
                                        "stop:1 rgba(230, 230, 230, 255));"
                                        "border: 1px solid rgb(200, 200, 200);"
                                        "border-radius: 4px;"
                                        "padding-left:%2px;}"
                                        "QDateEdit::drop-down{width:%1px;border-image: url(:/Images/triangledown.png);}")
                                    .arg(35).arg(10));
 

    m_pStartTimerEdit->setCalendarWidget(m_pStartCalendarWidget);    
    m_pStartCalendarWidget->setLocale(QLocale(QLocale::Chinese));
    m_pStartCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_pStartCalendarWidget->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);    
    m_pStartCalendarWidget->setFixedSize(500,400);
    m_pStartCalendarWidget->setFont(font);

	QString style="QCalendarWidget QWidget#qt_calendar_navigationbar {height:40px;};QMenu { font-size:10px; width: 150px; left: 20px;"
	"background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}"
	"QToolButton {icon-size: 48px, 48px;background-color: qlineargradient(x1:0, y1:0, x2:0,"
	"y2:1, stop: 0 #cccccc, stop: 1 #333333);"
	"height: 100px; width: 200px;}"
	"QAbstractItemView {selection-background-color: rgb(255, 174, 0);}"
	"QToolButton::menu-arrow {}"
	"QToolButton::menu-button {}"
	"QToolButton::menu-indicator{width: 50px;}"
	"QToolButton::menu-indicator:pressed,"
	"QToolButton::menu-indicator:open{top:10px; left: 10px;}"
	"QListView {background-color:white;}"
	"QSpinBox::up-button { subcontrol-origin: border;"
	"subcontrol-position: top right; width:50px; border-image: url(icons:arrow_up_n.png);}"
	"QSpinBox::down-button {subcontrol-origin: border; subcontrol-position: bottom right;"
	"border-width: 1px; width:50px;}"
	"QSpinBox::down-arrow { width:26px; height:17px;"
	"image: url(icons:arrow_down_n.png); } ";
    m_pStartCalendarWidget->setStyleSheet(style);

    QWidget *calendarNavBar = m_pStartCalendarWidget->findChild<QWidget *>("qt_calendar_navigationbar");
    if (calendarNavBar) {
        calendarNavBar->setFixedSize(500,40);
    }

    m_pEndTimerEdit->setCalendarWidget(m_pEndCalendarWidget);    
    m_pEndCalendarWidget->setLocale(QLocale(QLocale::Chinese));
    m_pEndCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_pEndCalendarWidget->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);    
    m_pEndCalendarWidget->setFixedSize(500,400);
    m_pEndCalendarWidget->setFont(font);  
    m_pEndCalendarWidget->setStyleSheet(style); 
 

    QWidget *calendarNavBar2 = m_pEndCalendarWidget->findChild<QWidget *>("qt_calendar_navigationbar");
    if (calendarNavBar2) {
        calendarNavBar2->setFixedSize(500,40);
    }     

    m_pEndTimerEdit->setCalendarPopup(true); //使用该句可以直接调用日历控件
    m_pEndTimerEdit->setCalendarWidget(m_pEndCalendarWidget);
    m_pEndTimerEdit->setDisplayFormat("yyyy/MM/dd");//yyyy/MM/dd
    m_pEndTimerEdit->setFont(font);
    m_pEndTimerEdit->setStyleSheet(QString("QDateEdit{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
                                        "stop:0 rgba(250, 250, 250, 255), "
                                        "stop:0.5 rgba(240, 240, 240, 255), "
                                        "stop:1 rgba(230, 230, 230, 255));"
                                        "border: 1px solid rgb(200, 200, 200);"
                                        "border-radius: 4px;"
                                        "padding-left:%2px;}"
                                        "QDateEdit::drop-down{width:%1px;border-image: url(:/Images/triangledown.png);}")
                                    .arg(35).arg(10));
    m_pEndTimerEdit->setDate(QDate::currentDate());   

    m_pStartTimerEdit->setFixedSize(230, 52);
    m_pEndTimerEdit->setFixedSize(230, 52);

    m_pQueryButton->setFixedSize(60, 52);//100
    m_pExportButton->setFixedSize(60, 52);
    m_pCleanButton->setFixedSize(60, 52);
}

void ViewAccessRecordsFrmPrivate::InitConnect()
{
    QObject::connect(m_pQueryButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotQueryButton);
    QObject::connect(m_pPageNavigator, &PageNavigator::currentPageChanged, q_func(), &ViewAccessRecordsFrm::slotCurrentPageChanged);
    QObject::connect(m_pExportButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotExportButton);
    QObject::connect(m_pCleanButton, &QPushButton::clicked, q_func(), &ViewAccessRecordsFrm::slotCleanButton);
}

void ViewAccessRecordsFrmPrivate::SelectData(const QString &sql)
{
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    query.prepare(sql);
    query.exec();
    while (query.next())
    {//取出对应数据
#ifdef Q_OS_WIN
        this->AddProjectInfo(query.value("name").toString(), query.value("sex").toString(), query.value("tempvalue").toString(), query.value("time").toString(), "C:\\Users\\63279\\face_crop_image\\2021-10-20\\202110201951474_9.jpg");
#else
        this->AddProjectInfo(query.value("name").toString(), query.value("sex").toString(), query.value("tempvalue").toString(), query.value("time").toString(), query.value("img_path").toString());
#endif
    }
}

void ViewAccessRecordsFrmPrivate::ClearTableData()
{
    this->m_pTableViewModel->removeRows(0, this->m_pTableViewModel->rowCount());
}

void ViewAccessRecordsFrmPrivate::AddProjectInfo(const QString &name, const QString &sex, const QString &tempvalue, const QString &time, const QString &img_path)
{
    QList<QStandardItem*> list;
    list << new QStandardItem(name) << new QStandardItem(sex) << new QStandardItem(QString::number(tempvalue.toFloat(), 'f', 2)) << new QStandardItem(time) <<new QStandardItem("");
    m_pTableViewModel->appendRow(list);
    int nRowCount = m_pTableViewModel->rowCount() - 1;
    m_pTableView->setRowHeight(nRowCount, 110);
    m_pTableView->setColumnWidth(0, 120);
    m_pTableView->setColumnWidth(1, 60);
    m_pTableView->setColumnWidth(2, 120); 
    m_pTableView->setColumnWidth(3, 180);   
    m_pTableView->setColumnWidth(m_pTableViewModel->columnCount() - 1, 260);

    m_pTableViewModel->item(nRowCount, 1)->setTextAlignment(Qt::AlignCenter);
    m_pTableViewModel->item(nRowCount, 2)->setTextAlignment(Qt::AlignCenter);
    m_pTableViewModel->item(nRowCount, m_pTableViewModel->columnCount() - 1)->setTextAlignment(Qt::AlignCenter);

    QLabel *pngLabel = new QLabel;
    QPixmap png = QPixmap(img_path);    
#if 0    
    if(!png.isNull())png = png.scaled(260, 110);
    pngLabel->setScaledContents(true);    
#else 
//QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充 拉伸
    pngLabel->setAlignment(Qt::AlignCenter);
   png.scaled(260, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
#endif     
    pngLabel->setPixmap(png);
    m_pTableView->setIndexWidget(m_pTableViewModel->index(nRowCount, m_pTableViewModel->columnCount() - 1), pngLabel);
}

static inline QString PackLikeselect(const QString &begin, const QString &end)
{
    if(begin.isEmpty() && end.isEmpty())return QString("select * from identifyrecord");
    else if(begin.isEmpty() && !end.isEmpty())return QString("select * from identifyrecord where time<='%1'").arg(end);
    else if(!begin.isEmpty() && end.isEmpty())return QString("select * from identifyrecord where time>='%1'").arg(begin);
    return QString("select * from identifyrecord where time>='%1' and time<='%2'").arg(begin).arg(end);
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
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));

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

void ViewAccessRecordsFrm::setEnter()
{
    Q_D(ViewAccessRecordsFrm);
    d->m_pQueryButton->setFocus();
    this->slotQueryButton();

    static bool Init = false;
    if(!Init)
    {
        Init = true;
        QObject::connect(qXLApp->GetRecordsExport(), &RecordsExport::sigExportProgressShell, this, &ViewAccessRecordsFrm::slotExportProgressShell);
        QObject::connect(this, &ViewAccessRecordsFrm::sigExportPersons, qXLApp->GetRecordsExport(), &RecordsExport::slotExportPersons);
    }
}

void ViewAccessRecordsFrm::slotCurrentPageChanged(const int page)
{
    Q_D(ViewAccessRecordsFrm);
    Q_UNUSED(page);
    d->m_pQueryButton->setFocus();
    d->ClearTableData();
    QString text = ::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59");
    d->SelectData((text + QString(" order by rid desc LIMIT %1,%2").arg((page - 1) *PAGE).arg(PAGE)));
}

void ViewAccessRecordsFrm::slotQueryButton()
{
    Q_D(ViewAccessRecordsFrm);
    d->ClearTableData();
    QString text = ::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59");
    d->m_pPageNavigator->setMaxPage(SelectDataCount(text));
    d->SelectData((text + QString(" order by rid desc LIMIT 0,%1").arg(PAGE)));
}

void ViewAccessRecordsFrm::slotExportButton()
{
    Q_D(ViewAccessRecordsFrm);
#ifdef Q_OS_LINUX
    //检查U盘是否插入
    bool usbState = UsbObserver::GetInstance()->isUsbStroagePlugin();
    if(usbState)
    {
        d->m_pExportButton->setEnabled(false);
        d->m_pExportButton->setText(QObject::tr("Export..."));//导出中...
		d->m_pProgressDialog = new QProgressDialog(this);
		d->m_pProgressDialog->setWindowModality(Qt::WindowModal);
		d->m_pProgressDialog->setAttribute(Qt::WA_DeleteOnClose,true);
		d->m_pProgressDialog->setMinimumDuration(5);//5
		d->m_pProgressDialog->setWindowTitle(QObject::tr("PlsWaiting"));//请稍等
		d->m_pProgressDialog->setLabelText(QObject::tr("Export..."));//导出中...
		d->m_pProgressDialog->setCancelButtonText(NULL);		
        emit sigExportPersons(::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59"));
    }else
    {
        OperationTipsFrm dlg(this);
        //dlg.setMessageBox(QObject::tr("温馨提示"), QObject::tr("请插入U盘"), QObject::tr("确定"), QString(), 1);
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("PlsInsertUDisk"), QObject::tr("Ok"), QString(), 1);
    }
#else

    d->m_pExportButton->setEnabled(false);
    d->m_pExportButton->setText(QObject::tr("Export..."));//导出中...
    emit sigExportPersons(::PackLikeselect(d->m_pStartTimerEdit->text()+" 00:00:00", d->m_pEndTimerEdit->text()+" 23:59:59"));
#endif
}

void ViewAccessRecordsFrm::slotCleanButton()
{
    Q_D(ViewAccessRecordsFrm);
	char cmdline[256];
    QMessageBox::StandardButton btn;
    //btn = QMessageBox::question(this, "提示", "此操作请小心慎重,会清除全部通行数据(包括数据与图片),确实要删除吗?",
    btn = QMessageBox::question(this, QObject::tr("Tips"), QObject::tr("PurgeHint"),
        QMessageBox::Yes|QMessageBox::No);
    if (btn ==QMessageBox::Yes)   {

        //清除记录
        sprintf(cmdline,"%s","sqlite3 /mnt/user/facedb/isc_ir_arcsoft_face.db 'delete from identifyrecord ';");	  
        qDebug()<<"清除记录 cmdline: "<<cmdline;
        system(cmdline);

	    //清除图片
	    sprintf(cmdline,"find /mnt/user/face_crop_image/ -name *.jpg  | awk  ' {print $0 } ' | xargs rm -rf ");	  	 
	    qDebug()<<"清除图片 cmdline: "<<cmdline;
	
	    system(cmdline);

        d->ClearTableData();
    }
}

void ViewAccessRecordsFrm::slotExportProgressShell(const bool dealstate, const bool savestate, const int total, const int dealcnt)
{
    Q_D(ViewAccessRecordsFrm);
    Q_UNUSED(savestate);
    Q_UNUSED(total);
    Q_UNUSED(dealcnt);
    if(dealstate)
    {
		d->m_pProgressDialog->setRange(0, total);	
		d->m_pProgressDialog->setValue(dealcnt);
		d->m_pProgressDialog->setModal(true); 			
		d->m_pProgressDialog->show();
		
		if (total == dealcnt)
		{
			
			QTime dieTime = QTime::currentTime().addMSecs(500);

			while( QTime::currentTime() < dieTime )
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

		
			d->m_pExportButton->setEnabled(true);
			d->m_pExportButton->setText(QObject::tr("Export"));//导出
			
			d->m_pProgressDialog->close();
			d->m_pProgressDialog = NULL;
		
		}
    }
}

void ViewAccessRecordsFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
