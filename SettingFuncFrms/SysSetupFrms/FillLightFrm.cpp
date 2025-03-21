#include "FillLightFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

class FillLightFrmPrivate
{
    Q_DECLARE_PUBLIC(FillLightFrm)
public:
    FillLightFrmPrivate(FillLightFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pFillLightGroup;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    FillLightFrm *const q_ptr;
};

FillLightFrmPrivate::FillLightFrmPrivate(FillLightFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FillLightFrm::FillLightFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new FillLightFrmPrivate(this))
{

}

FillLightFrm::~FillLightFrm()
{

}

void FillLightFrmPrivate::InitUI()
{
    m_pFillLightGroup = new QButtonGroup;
    m_pFillLightGroup->addButton(new QRadioButton, 0);//常闭
    m_pFillLightGroup->addButton(new QRadioButton, 1);//常开
    m_pFillLightGroup->addButton(new QRadioButton, 2);//自动

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("FillLightSetting")));//补光设置
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(10);
    layout1->addWidget(new QLabel(QObject::tr("NormallyClosed")));//常闭
    layout1->addStretch();
    layout1->addWidget(m_pFillLightGroup->button(0));
    layout1->addSpacing(10);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(10);
    layout2->addWidget(new QLabel(QObject::tr("NormallyOpen")));//常开
    layout2->addStretch();
    layout2->addWidget(m_pFillLightGroup->button(1));
    layout2->addSpacing(10);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addSpacing(10);
    layout3->addWidget(new QLabel(QObject::tr("Auto")));//自动
    layout3->addStretch();
    layout3->addWidget(m_pFillLightGroup->button(2));
    layout3->addSpacing(10);

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

    vlayout->addLayout(layout);

    {
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

void FillLightFrmPrivate::InitData()
{
    q_func()->setObjectName("FillLightFrm");

    m_pFillLightGroup->button(0)->setChecked(true);
    m_pFillLightGroup->button(0)->setObjectName("choiceRadioButton");
    m_pFillLightGroup->button(1)->setObjectName("choiceRadioButton");
    m_pFillLightGroup->button(2)->setObjectName("choiceRadioButton");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void FillLightFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void FillLightFrm::setFillLightMode(const int &index)
{
    Q_D(FillLightFrm);
    d->m_pFillLightGroup->button(index)->setChecked(true);
}

int FillLightFrm::getFillLightMode() const
{
    return d_func()->m_pFillLightGroup->checkedId();
}

void FillLightFrm::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if(event->pos().y()>=47 && event->pos().y()<=97)
        this->setFillLightMode(0);
    else if(event->pos().y()>=100.0 && event->pos().y()<150)
        this->setFillLightMode(1);
    else if(event->pos().y()>=150 && event->pos().y()<=200)
        this->setFillLightMode(2);
#else
    if(event->pos().y()>=68 && event->pos().y()<=128)
        this->setFillLightMode(0);
    else if(event->pos().y()>=137 && event->pos().y()<200)
        this->setFillLightMode(1);
    else if(event->pos().y()>=209 && event->pos().y()<=271)
        this->setFillLightMode(2);
#endif
    QDialog::mousePressEvent(event);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void FillLightFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif