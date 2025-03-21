#include "Toast.h"
#include "SharedInclude/GlobalDef.h"

Toast::Toast() :
		mLabel(ISC_NULL), mTimer(ISC_NULL), mKeepSecond(0)
{
}

Toast::~Toast()
{
}

void Toast::showTips(QRect rect, QString msg, int nKeepSecond)
{
	Toast* thiz = new Toast;
	QColor color = QColor(0x00, 0x00, 0x00, 0x00);
	QPalette palette;
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Background, color);
	QString css("QLabel#ToastLabel{background-color: #66000000;font-size:22px;}");

	thiz->mKeepSecond = nKeepSecond;
	thiz->mLabel = new QLabel;
	thiz->mLabel->setPalette(palette);
	thiz->mLabel->setAutoFillBackground(true);
	thiz->mLabel->setAlignment(Qt::AlignCenter);

	thiz->mLabel->setObjectName("ToastLabel");
	thiz->mLabel->setStyleSheet(css);
	thiz->mLabel->setGeometry(rect);
	thiz->mLabel->setText(msg);
	thiz->mLabel->show();

	thiz->mTimer = new QTimer(thiz);
	thiz->mTimer->setInterval(100);
	thiz->mTimer->start();
	connect(thiz->mTimer, SIGNAL(timeout()), thiz, SLOT(updateTips()));
}

void Toast::showTips(QRect rect, QString msg, int fontSize, int nKeepSecond)
{
	Toast* thiz = new Toast;
	QColor color = QColor(0x00, 0x00, 0x00, 0x00);
	QPalette palette;
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Background, color);
	QString css = "QLabel#ToastLabel{background-color: #66000000;font-size:"+QString::number(fontSize)+"px;}";

	thiz->mKeepSecond = nKeepSecond;
	thiz->mLabel = new QLabel;
	thiz->mLabel->setPalette(palette);
	thiz->mLabel->setAutoFillBackground(true);
	thiz->mLabel->setAlignment(Qt::AlignCenter);

	thiz->mLabel->setObjectName("ToastLabel");
	thiz->mLabel->setStyleSheet(css);
	thiz->mLabel->setGeometry(rect);
	thiz->mLabel->setText(msg);
	thiz->mLabel->show();

	thiz->mTimer = new QTimer(thiz);
	thiz->mTimer->setInterval(100);
	thiz->mTimer->start();
	connect(thiz->mTimer, SIGNAL(timeout()), thiz, SLOT(updateTips()));
}

void Toast::updateTips()
{
	if (mKeepSecond-- <= 0)
	{
		if (mLabel != ISC_NULL && mLabel->isVisible())
		{
			mLabel->hide();
			delete mLabel;
		}

		if (mTimer != ISC_NULL)
		{
			disconnect(this, 0, 0, 0);
			mTimer->stop();
			delete mTimer;
			mTimer = ISC_NULL;
		}
		delete this;
	}
}

