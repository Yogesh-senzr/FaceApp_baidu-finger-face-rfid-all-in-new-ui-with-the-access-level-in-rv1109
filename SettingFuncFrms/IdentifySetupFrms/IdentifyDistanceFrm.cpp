#include "IdentifyDistanceFrm.h"

#include "Config/ReadConfig.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

class IdentifyDistanceFrmPrivate
{
    Q_DECLARE_PUBLIC(IdentifyDistanceFrm)
public:
    IdentifyDistanceFrmPrivate(IdentifyDistanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pDistanceGroup;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    IdentifyDistanceFrm *const q_ptr;
};

IdentifyDistanceFrmPrivate::IdentifyDistanceFrmPrivate(IdentifyDistanceFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

IdentifyDistanceFrm::IdentifyDistanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new IdentifyDistanceFrmPrivate(this))
{

}

IdentifyDistanceFrm::~IdentifyDistanceFrm()
{

}

void IdentifyDistanceFrmPrivate::InitUI()
{
    m_pDistanceGroup = new QButtonGroup;
    m_pDistanceGroup->addButton(new QRadioButton, 0);//50
    m_pDistanceGroup->addButton(new QRadioButton, 1);//100
    m_pDistanceGroup->addButton(new QRadioButton, 2);//150

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("DistanceOfRecognition")));//识别距离
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(20);
    layout1->addWidget(new QLabel(QObject::tr("50cm")));
    layout1->addStretch();
    layout1->addWidget(m_pDistanceGroup->button(0));
    layout1->addSpacing(20);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(20);
    layout2->addWidget(new QLabel(QObject::tr("100cm")));
    layout2->addStretch();
    layout2->addWidget(m_pDistanceGroup->button(1));
    layout2->addSpacing(20);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addSpacing(20);
    layout3->addWidget(new QLabel(QObject::tr("150cm")));
    layout3->addStretch();
    layout3->addWidget(m_pDistanceGroup->button(2));
    layout3->addSpacing(20);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->addSpacing(10);
    layout5->addWidget(m_pConfirmButton);
    layout5->addWidget(m_pCancelButton);
    layout5->addSpacing(10);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    layout1->itemAt(1)->widget()->setObjectName("DialogLabel");
    layout2->itemAt(1)->widget()->setObjectName("DialogLabel");
    layout3->itemAt(1)->widget()->setObjectName("DialogLabel");


    QVBoxLayout *vlayout= new QVBoxLayout(q_func());
    vlayout->setContentsMargins(0, 5, 0, 15);
    {
        vlayout->addLayout(layout);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
    {
        vlayout->addLayout(layout1);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
    {
        vlayout->addLayout(layout2);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
    {
        vlayout->addLayout(layout3);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }


    vlayout->addLayout(layout5);
}

void IdentifyDistanceFrmPrivate::InitData()
{
    q_func()->setObjectName("IdentifyDistanceFrm");

    //m_pDistanceGroup->button(1)->setChecked(true);
    int index = ReadConfig::GetInstance()->getIdentity_Manager_Identifycm();
    m_pDistanceGroup->button(index)->setChecked(true);
    m_pDistanceGroup->button(0)->setObjectName("choiceRadioButton");
    m_pDistanceGroup->button(1)->setObjectName("choiceRadioButton");
    m_pDistanceGroup->button(2)->setObjectName("choiceRadioButton");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void IdentifyDistanceFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void IdentifyDistanceFrm::setDistanceMode(const int &index)
{
    Q_D(IdentifyDistanceFrm);
    d->m_pDistanceGroup->button(index)->setChecked(true);
}

int IdentifyDistanceFrm::getDistanceMode() const
{
    return d_func()->m_pDistanceGroup->checkedId();
}

void IdentifyDistanceFrm::mousePressEvent(QMouseEvent *event)
{
	printf(">>>>%s,%s,%d,.y()=%d\n",__FILE__,__func__,__LINE__,event->pos().y());
	if (QApplication::desktop()->screenGeometry().width()>=800)
	{
		if(event->pos().y()>=68 && event->pos().y()<=128)
			this->setDistanceMode(0);
		else if(event->pos().y()>=137 && event->pos().y()<200)
			this->setDistanceMode(1);
		else if(event->pos().y()>=209 && event->pos().y()<=271)
			this->setDistanceMode(2);
	}
	else if (QApplication::desktop()->screenGeometry().width()>=720)
	{
		if(event->pos().y()>=68 && event->pos().y()<=128)
			this->setDistanceMode(0);
		else if(event->pos().y()>=137 && event->pos().y()<200)
			this->setDistanceMode(1);
		else if(event->pos().y()>=209 && event->pos().y()<=271)
			this->setDistanceMode(2);
	}
	else if (QApplication::desktop()->screenGeometry().width()>=600)
	{
		if(event->pos().y()>=68 && event->pos().y()<=128)
			this->setDistanceMode(0);
		else if(event->pos().y()>=137 && event->pos().y()<200)
			this->setDistanceMode(1);
		else if(event->pos().y()>=209 && event->pos().y()<=271)
			this->setDistanceMode(2);
	}	
	else if (QApplication::desktop()->screenGeometry().width()>=480)
	{
		if(event->pos().y()>=68 && event->pos().y()<=128)
			this->setDistanceMode(0);
		else if(event->pos().y()>=137 && event->pos().y()<200)
			this->setDistanceMode(1);
		else if(event->pos().y()>=209 && event->pos().y()<=271)
			this->setDistanceMode(2);		
	}

    QDialog::mousePressEvent(event);
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void IdentifyDistanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif