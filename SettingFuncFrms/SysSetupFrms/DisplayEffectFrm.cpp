#include "DisplayEffectFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>

class DisplayEffectFrmPrivate
{
    Q_DECLARE_PUBLIC(DisplayEffectFrm)
public:
    DisplayEffectFrmPrivate(DisplayEffectFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    DisplayEffectFrm *const q_ptr;
};

DisplayEffectFrmPrivate::DisplayEffectFrmPrivate(DisplayEffectFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

DisplayEffectFrm::DisplayEffectFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new DisplayEffectFrmPrivate(this))
{

}

DisplayEffectFrm::~DisplayEffectFrm()
{

}

void DisplayEffectFrmPrivate::InitUI()
{
    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("DisplayEffect")));//显示效果
    layout->addStretch();

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addSpacing(10);
    hlayout2->addWidget(m_pConfirmButton);
    hlayout2->addWidget(m_pCancelButton);
    hlayout2->addSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(0, 5, 0, 10);
    vLayout->addLayout(layout);
    vLayout->addSpacing(10);
    vLayout->addLayout(hlayout2);
}

void DisplayEffectFrmPrivate::InitData()
{
    q_func()->setObjectName("DisplayEffectFrm");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void DisplayEffectFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void DisplayEffectFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 