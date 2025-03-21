#pragma once

#include <QDialog>
//操作提示
class OperationTipsFrmPrivate;
class OperationTipsFrm : public QDialog
{
	Q_OBJECT
public:
	OperationTipsFrm(QWidget *parent = Q_NULLPTR);
	~OperationTipsFrm();
public://btnshow(0不显示，1显左边，2显右边，3全显) //QString("确定"),QString("取消")
	int setMessageBox(const QString &title, const QString &text, const QString &confirm = QObject::tr("Ok"), const QString &cancel =QObject::tr("Cancel") , const int btnshow = 3);
private:
	QScopedPointer<OperationTipsFrmPrivate>d_ptr;
private:
	Q_DECLARE_PRIVATE(OperationTipsFrm)
	Q_DISABLE_COPY(OperationTipsFrm)
};
