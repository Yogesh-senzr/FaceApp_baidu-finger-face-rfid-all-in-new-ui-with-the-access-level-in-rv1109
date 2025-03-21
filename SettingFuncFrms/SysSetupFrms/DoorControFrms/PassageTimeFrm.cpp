#include "PassageTimeFrm.h"

#include "Delegate/wholedelegate.h"
#include "Delegate/Timedelegate.h"
#include "Delegate/CheckBoxDelegate.h"

#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlRecord>

class PassageTimeFrmPrivate
{
    Q_DECLARE_PUBLIC(PassageTimeFrm)
public:
    PassageTimeFrmPrivate(PassageTimeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QWidget *newCheckBox(const int &state);
private:
    void DeleteAlldbData();
private:
    QPushButton *m_pAddTimeIntervalBtn;//增加时段
    QPushButton *m_pDelTimeIntervalBtn;//删除时段
    QTableWidget *m_pTableWidget;
private:
    PassageTimeFrm *const q_ptr;
};

PassageTimeFrmPrivate::PassageTimeFrmPrivate(PassageTimeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

PassageTimeFrm::PassageTimeFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new PassageTimeFrmPrivate(this))
{

}

PassageTimeFrm::~PassageTimeFrm()
{

}

void PassageTimeFrmPrivate::InitUI()
{
    m_pAddTimeIntervalBtn = new QPushButton;//增加时段
    m_pDelTimeIntervalBtn = new QPushButton;//删除时段

    m_pTableWidget = new QTableWidget;


    QHBoxLayout *btnlayout = new QHBoxLayout;
    btnlayout->addSpacing(20);
    btnlayout->addWidget(m_pAddTimeIntervalBtn);
    btnlayout->addWidget(m_pDelTimeIntervalBtn);
    btnlayout->addStretch();

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->addLayout(btnlayout);
    malayout->addWidget(m_pTableWidget);
}

void PassageTimeFrmPrivate::InitData()
{
    QStringList tableHeader;
    //tableHeader << QObject::tr(" 开始时间") << QObject::tr("结束时间") << QObject::tr("星期一") << QObject::tr("星期二") << QObject::tr("星期三") << QObject::tr("星期四") << QObject::tr("星期五") << QObject::tr("星期六") << QObject::tr("星期日 ");
    tableHeader << QObject::tr("StartTime") << QObject::tr("EndTime") << QObject::tr("Monday") << QObject::tr("Tuesday") << QObject::tr("Wednesday") << QObject::tr("Thursday") << QObject::tr("Friday") << QObject::tr("Saturday") << QObject::tr("Sunday");
    m_pTableWidget->setColumnCount(tableHeader.count());
    m_pTableWidget->setHorizontalHeaderLabels(tableHeader);
    m_pTableWidget->verticalHeader()->hide();
    //m_pTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);// 自适应宽
    m_pTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);// 自适应宽
    m_pTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);// 自适应宽
    m_pTableWidget->setColumnWidth(0, 120);
    m_pTableWidget->setColumnWidth(1, 120);
    m_pTableWidget->setColumnWidth(2, 80);
    m_pTableWidget->setColumnWidth(3, 80);
    m_pTableWidget->setColumnWidth(4, 80);
    m_pTableWidget->setColumnWidth(5, 80);
    m_pTableWidget->setColumnWidth(6, 80);
    m_pTableWidget->setColumnWidth(7, 80);
    m_pTableWidget->setColumnWidth(8, 80);

    m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    //这里是去掉那边虚线
    m_pTableWidget->setItemDelegate(new wholeDelegate);
    //时间
    m_pTableWidget->setItemDelegateForColumn(0, new TimeDelegate);
    m_pTableWidget->setItemDelegateForColumn(1, new TimeDelegate);

    m_pAddTimeIntervalBtn->setText(QObject::tr("AddPeriod"));//增加时段
    m_pDelTimeIntervalBtn->setText(QObject::tr("DeletePeriod"));//删除时段

    m_pAddTimeIntervalBtn->setFixedSize(128, 46);
    m_pDelTimeIntervalBtn->setFixedSize(128, 46);
}

void PassageTimeFrmPrivate::InitConnect()
{
    QObject::connect(m_pAddTimeIntervalBtn, &QPushButton::clicked, q_func(), &PassageTimeFrm::slotAddTimeInterval);
    QObject::connect(m_pDelTimeIntervalBtn, &QPushButton::clicked, q_func(), &PassageTimeFrm::slotDelTimeInterval);
}

QWidget *PassageTimeFrmPrivate::newCheckBox(const int &state)
{
    QCheckBox *check = new QCheckBox;
    check->setChecked(state ? true : false);
    check->setObjectName("FCheckBox");
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(check);
    layout->addStretch();

    QWidget *f = new QWidget;
    f->setLayout(layout);
    return f;
}

void PassageTimeFrmPrivate::DeleteAlldbData()
{
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("delete from PassageTime");
    query.exec();
}

void PassageTimeFrm::setEnter()
{
    Q_D(PassageTimeFrm);
    d->m_pTableWidget->setRowCount(0);
    d->m_pTableWidget->clearContents();

    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("select *from PassageTime");
    query.exec();
    while(query.next())
    {
        int row = d->m_pTableWidget->rowCount();
        d->m_pTableWidget->insertRow(row);
        d->m_pTableWidget->setRowHeight(row, 72);

        QStringList msg;
        msg << query.value("stateTimer").toString() << query.value("endTimer").toString() << "" << "" << "" << "" << "" << "" << "";
        for (int i = 0; i < msg.count(); i++)
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setText(msg.at(i));
            item->setTextAlignment(Qt::AlignCenter);
            d->m_pTableWidget->setItem(row, i, item);
        }
        d->m_pTableWidget->setCellWidget(row, 2, d->newCheckBox(query.value("Monday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 3, d->newCheckBox(query.value("Tuesday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 4, d->newCheckBox(query.value("Wednesday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 5, d->newCheckBox(query.value("Thursday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 6, d->newCheckBox(query.value("Friday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 7, d->newCheckBox(query.value("Saturday").toInt()));
        d->m_pTableWidget->setCellWidget(row, 8, d->newCheckBox(query.value("Sunday").toInt()));
    }
}

void PassageTimeFrm::setLeaveEvent()
{
    Q_D(PassageTimeFrm);
    d->DeleteAlldbData();
    int row = d->m_pTableWidget->rowCount();
    for(int i = 0; i<row; i++)
    {
        QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
        QString strSql;
        strSql.append("INSERT OR REPLACE INTO PassageTime");
        strSql.append("(stateTimer,");
        strSql.append("endTimer,");
        strSql.append("Monday,");
        strSql.append("Tuesday,");
        strSql.append("Wednesday,");
        strSql.append("Thursday,");
        strSql.append("Friday,");
        strSql.append("Saturday,");
        strSql.append("Sunday)");
        strSql.append("VALUES(?,?,?,?,?,?,?,?,?)");
        query.prepare(strSql);

        query.bindValue(0, d->m_pTableWidget->item(i, 0)->text());
        query.bindValue(1, d->m_pTableWidget->item(i, 1)->text());
        query.bindValue(2, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 2)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(3, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 3)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(4, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 4)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(5, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 5)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(6, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 6)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(7, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 7)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.bindValue(8, ((QCheckBox *)d->m_pTableWidget->cellWidget(i, 8)->layout()->itemAt(1)->widget())->checkState() ? 1 : 0);
        query.exec();
    }
}

void PassageTimeFrm::slotAddTimeInterval()
{
    Q_D(PassageTimeFrm);

    int row = d->m_pTableWidget->rowCount();
    d->m_pTableWidget->insertRow(row);
    d->m_pTableWidget->setRowHeight(row, 72);

    QStringList msg;
    msg << "00:00" << "23:59" << "" << "" << "" << "" << "" << "" << "";
    for (int i = 0; i < msg.count(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(msg.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        d->m_pTableWidget->setItem(row, i, item);

        if(i>1)d->m_pTableWidget->setCellWidget(row, i, d->newCheckBox(1));
    }
}

void PassageTimeFrm::slotDelTimeInterval()
{
    Q_D(PassageTimeFrm);
    int row = d->m_pTableWidget->currentRow();
    if (row == -1)return;
    d->m_pTableWidget->removeRow(row);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void PassageTimeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 