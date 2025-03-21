#ifndef RECORDSEXPORT_H
#define RECORDSEXPORT_H

#include <QObject>

class RecordsExport : public QObject
{
    Q_OBJECT
public:
    explicit RecordsExport(QObject *parent = nullptr);
public:
    Q_SLOT void slotExportPersons(const QString);
public:
    //处理导出进度(导出进度， 保存状态)
    Q_SIGNAL void sigExportProgressShell(const bool, const bool, const int total, const int dealcnt);
};

#endif // RECORDSEXPORT_H
