#ifndef _INCLUDE_WIDGETS_TOAST_H_
#define _INCLUDE_WIDGETS_TOAST_H_

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtGui/QPalette>
#include <QtWidgets/QLabel>

class Toast: public QObject
{
Q_OBJECT

public:
	Toast();
	~Toast();

public:
	static void showTips(QRect rect, QString msg, int nKeepSecond);
	static void showTips(QRect rect, QString msg, int fontSize, int nKeepSecond);
public:
	QLabel *mLabel;
	QTimer *mTimer;
	int mKeepSecond;

private slots:
	void updateTips();
};

#endif /* _INCLUDE_WIDGETS_TOAST_H_ */
