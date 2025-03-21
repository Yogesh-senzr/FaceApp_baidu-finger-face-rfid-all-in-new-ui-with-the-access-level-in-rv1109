#include "AccessTypeFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

class AccessTypeFrmPrivate
{
    Q_DECLARE_PUBLIC(AccessTypeFrm)
public:
    AccessTypeFrmPrivate(AccessTypeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pAccessTypeGroup;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    AccessTypeFrm *const q_ptr;
};

AccessTypeFrmPrivate::AccessTypeFrmPrivate(AccessTypeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

AccessTypeFrm::AccessTypeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new AccessTypeFrmPrivate(this))
{

}

AccessTypeFrm::~AccessTypeFrm()
{

}

void AccessTypeFrmPrivate::InitUI()
{
    m_pAccessTypeGroup = new QButtonGroup;
    m_pAccessTypeGroup->addButton(new QRadioButton, 0);//Entry
    m_pAccessTypeGroup->addButton(new QRadioButton, 1);//Exit
    m_pAccessTypeGroup->addButton(new QRadioButton, 2);//EntryExit

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("AccessType")));//出入类型
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(10);
    layout1->addWidget(new QLabel(QObject::tr("GateEntry")));//入闸
    layout1->addStretch();
    layout1->addWidget(m_pAccessTypeGroup->button(0));
    layout1->addSpacing(10);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(10);
    layout2->addWidget(new QLabel(QObject::tr("ExitGate")));//出闸
    layout2->addStretch();
    layout2->addWidget(m_pAccessTypeGroup->button(1));
    layout2->addSpacing(10);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addSpacing(10);
    layout3->addWidget(new QLabel(QObject::tr("EntryExitGate")));//入出闸
    layout3->addStretch();
    layout3->addWidget(m_pAccessTypeGroup->button(2));
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

void AccessTypeFrmPrivate::InitData()
{
    q_func()->setObjectName("AccessTypeFrm");

    m_pAccessTypeGroup->button(0)->setChecked(true);
    m_pAccessTypeGroup->button(0)->setObjectName("choiceRadioButton");
    m_pAccessTypeGroup->button(1)->setObjectName("choiceRadioButton");
    m_pAccessTypeGroup->button(2)->setObjectName("choiceRadioButton");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void AccessTypeFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void AccessTypeFrm::setAccessType(const int &index)
{
    Q_D(AccessTypeFrm);
    if(index >= 0 && index <= 2) {
        d->m_pAccessTypeGroup->button(index)->setChecked(true);
    }
}

int AccessTypeFrm::getAccessType() const
{
    return d_func()->m_pAccessTypeGroup->checkedId();
}

void AccessTypeFrm::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if(event->pos().y()>=47 && event->pos().y()<=97)
        this->setAccessType(0);
    else if(event->pos().y()>=100.0 && event->pos().y()<150)
        this->setAccessType(1);
    else if(event->pos().y()>=153.0 && event->pos().y()<203)
        this->setAccessType(2);
#else
    if(event->pos().y()>=68 && event->pos().y()<=128)
        this->setAccessType(0);
    else if(event->pos().y()>=137 && event->pos().y()<200)
        this->setAccessType(1);
    else if(event->pos().y()>=209 && event->pos().y()<272)
        this->setAccessType(2);
#endif
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void AccessTypeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif
