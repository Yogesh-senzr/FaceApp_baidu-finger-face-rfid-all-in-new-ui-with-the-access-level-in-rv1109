#ifndef PARSEPERSONXLSX_H
#define PARSEPERSONXLSX_H

#include <QObject>

class ParsePersonXlsxPrivate;
class ParsePersonXlsx : public QObject
{
    Q_OBJECT
public:
    ParsePersonXlsx(QObject *parent = Q_NULLPTR);
    ~ParsePersonXlsx();
public:
    Q_SLOT void slotParseXlsxPath(const QString);//解析xlsx路径
    Q_SLOT void slotExportPersons();//导出人员信息

public slots:
    void slotAddFaceDataFromXlsx(const QString Path);

signals:
    void sigAddFaceProgressShell(bool isFinished, int total, int current, int successCount, int failCount);
    void sigFaceDataUpdated(const QString &uuid);
public:
    //处理进度（是否完成、总数量、当前处理条数、成功记录、失败记录）
    Q_SIGNAL void sigImportProgressShell(const bool, const int total, const int dealcnt, const int succeedcnt, const int failcnt);
    //处理导出进度(导出进度， 保存状态)
    Q_SIGNAL void sigExportProgressShell(const bool, const bool, const int total, const int dealcnt);
private:
    void ImportPersonToDB();
private:
    QScopedPointer<ParsePersonXlsxPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ParsePersonXlsx)
    Q_DISABLE_COPY(ParsePersonXlsx)
};

#endif // PARSEPERSONXLSX_H
