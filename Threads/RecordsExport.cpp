#include "RecordsExport.h"
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellrange.h"
#include "xlsxworksheet.h"
#include "xlsxworkbook_p.h"
#include "xlsxcellformula.h"

#include <QDebug>
#include <QDateTime>
#include <QHash>
#include <QThread>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QDateTime>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <dirent.h>


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
RecordsExport::RecordsExport(QObject *parent) : QObject(parent)
{
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

void RecordsExport::slotExportPersons(const QString sql)
{
    QXlsx::Document xlsx;
    xlsx.setColumnWidth(QString("%1%2").arg("G").arg(1), 20);
    xlsx.setColumnWidth(QString("%1%2").arg("H").arg(1), 15);

    xlsx.write(QString("%1%2").arg("A").arg(1), "姓名");
    xlsx.write(QString("%1%2").arg("B").arg(1), "性别");
    xlsx.write(QString("%1%2").arg("C").arg(1), "身份证");
    xlsx.write(QString("%1%2").arg("D").arg(1), "IC卡号");
    xlsx.write(QString("%1%2").arg("E").arg(1), "温度");
    xlsx.write(QString("%1%2").arg("F").arg(1), "消息");
    xlsx.write(QString("%1%2").arg("G").arg(1), "通行时间");
    xlsx.write(QString("%1%2").arg("H").arg(1), "图片");

    int index = 2;
    int writeCnt = 0;
	int totalCnt=0;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    query.prepare(sql);
    query.exec();
	if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        totalCnt = query.size();
    }
    else
    {
        totalCnt = queryRowCount(query);
    }
    printf(">>>>>>%s,%s,%d,totalCnt=%d,sql=%s\n",__FILE__,__func__,__LINE__,totalCnt,sql.toStdString().data());
    while (query.next())
    {//取出对应数据
        xlsx.setRowHeight(index, 60);
        xlsx.insertImage(index - 1, 7, QSize(100, 80), QImage(query.value("img_path").toString()));        
        xlsx.write(QString("%1%2").arg("A").arg(index), query.value("name"));
        xlsx.write(QString("%1%2").arg("B").arg(index), query.value("sex"));
        xlsx.write(QString("%1%2").arg("C").arg(index), query.value("idcardnum"));
        xlsx.write(QString("%1%2").arg("D").arg(index), query.value("iccardnum"));
        xlsx.write(QString("%1%2").arg("E").arg(index), query.value("tempvalue"));
        xlsx.write(QString("%1%2").arg("F").arg(index), query.value("msg"));
        xlsx.write(QString("%1%2").arg("G").arg(index), query.value("time"));
        //xlsx.insertImage(index - 1, 7, QSize(100, 80), QImage(query.value("img_path").toString()));
        //msleep(15*1000);
        sleep(1);
        index++;
        ++writeCnt;
        //emit sigExportProgressShell(false, false, -1, writeCnt);
        printf(">>>>>>%s,%s,%d,writeCnt=%d\n",__FILE__,__func__,__LINE__,writeCnt);
		emit sigExportProgressShell(true, false, totalCnt, writeCnt);
    }

#ifdef Q_OS_WIN
    bool saveState = xlsx.saveAs(QString("./%1_export_Records.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
#else
    bool saveState = xlsx.saveAs(QString("/udisk/%1_export_Records.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
    printf(">>>>>>%s,%s,%d,saveState=%d\n",__FILE__,__func__,__LINE__,saveState);
    if (saveState== false) 
    {
        system("umount /udisk/");
        system("mount /dev/sda1 /udisk/");
        saveState = xlsx.saveAs(QString("/udisk/%1_export_Records.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));        
    }
    printf(">>>>>>%s,%s,%d,saveState=%d\n",__FILE__,__func__,__LINE__,saveState);
    system("sync");
#endif
    //emit sigExportProgressShell(true, saveState, -1, writeCnt);
}
