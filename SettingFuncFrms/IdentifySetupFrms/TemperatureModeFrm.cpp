#include "TemperatureModeFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

class TemperatureModeFrmPrivate
{
    Q_DECLARE_PUBLIC(TemperatureModeFrm)
public:
    TemperatureModeFrmPrivate(TemperatureModeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pTemperatureModeGroup;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    TemperatureModeFrm *const q_ptr;
};

TemperatureModeFrmPrivate::TemperatureModeFrmPrivate(TemperatureModeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

TemperatureModeFrm::TemperatureModeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new TemperatureModeFrmPrivate(this))
{

}

TemperatureModeFrm::~TemperatureModeFrm()
{

}

void TemperatureModeFrmPrivate::InitUI()
{
    m_pTemperatureModeGroup = new QButtonGroup;
    m_pTemperatureModeGroup->addButton(new QRadioButton, 0);//室内
    m_pTemperatureModeGroup->addButton(new QRadioButton, 1);//户外

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("TemperatureMeasuringEnvironment")));//"测温环境"
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(10);
    layout1->addWidget(new QLabel(QObject::tr("interior")));//室内
    layout1->addStretch();
    layout1->addWidget(m_pTemperatureModeGroup->button(0));
    layout1->addSpacing(10);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(10);
    layout2->addWidget(new QLabel(QObject::tr("outdoor")));//户外
    layout2->addStretch();
    layout2->addWidget(m_pTemperatureModeGroup->button(1));
    layout2->addSpacing(10);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->addSpacing(10);
    layout5->addWidget(m_pConfirmButton);
    layout5->addWidget(m_pCancelButton);
    layout5->addSpacing(10);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    layout1->itemAt(1)->widget()->setObjectName("DialogLabel");
    layout2->itemAt(1)->widget()->setObjectName("DialogLabel");

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

    vlayout->addLayout(layout5);
}

void TemperatureModeFrmPrivate::InitData()
{
    q_func()->setObjectName("TemperatureModeFrm");

    m_pTemperatureModeGroup->button(0)->setChecked(true);
    m_pTemperatureModeGroup->button(0)->setObjectName("choiceRadioButton");
    m_pTemperatureModeGroup->button(1)->setObjectName("choiceRadioButton");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void TemperatureModeFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void TemperatureModeFrm::setTemperatureMode(const int &index)
{
    Q_D(TemperatureModeFrm);
    d->m_pTemperatureModeGroup->button(index)->setChecked(true);
}

int TemperatureModeFrm::getTemperatureMode() const
{
    return d_func()->m_pTemperatureModeGroup->checkedId();
}

void TemperatureModeFrm::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if(event->pos().y()>=47 && event->pos().y()<=97)
        this->setTemperatureMode(0);
    else if(event->pos().y()>=100.0 && event->pos().y()<150)
        this->setTemperatureMode(1);
#else
    if(event->pos().y()>=68 && event->pos().y()<=128)
        this->setTemperatureMode(0);
    else if(event->pos().y()>=137 && event->pos().y()<200)
        this->setTemperatureMode(1);
#endif
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture   
void TemperatureModeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif