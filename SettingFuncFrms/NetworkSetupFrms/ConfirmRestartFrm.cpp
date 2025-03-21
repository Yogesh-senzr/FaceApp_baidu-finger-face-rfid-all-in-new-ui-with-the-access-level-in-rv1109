#include "ConfirmRestartFrm.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ConfirmRestartFrmPrivate
{
    Q_DECLARE_PUBLIC(ConfirmRestartFrm)
public:
    ConfirmRestartFrmPrivate(ConfirmRestartFrm *dd);
    void InitUI();
    void InitData();
    void InitConnect();

private:
    QLabel *m_pNameLabel;
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;

    ConfirmRestartFrm *const q_ptr;
};

ConfirmRestartFrmPrivate::ConfirmRestartFrmPrivate(ConfirmRestartFrm *dd):q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ConfirmRestartFrm::ConfirmRestartFrm(QWidget *parent)
    : QDialog(parent,Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    ,d_ptr(new ConfirmRestartFrmPrivate(this))
{

}

ConfirmRestartFrm::~ConfirmRestartFrm()
{

}

void ConfirmRestartFrmPrivate::InitUI()
{
    m_pNameLabel = new QLabel;
    m_pConfirmButton = new QPushButton;
    m_pCancelButton = new QPushButton;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_pNameLabel);
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_pConfirmButton);
    layout1->addStretch(10);
    layout1->addWidget(m_pCancelButton);

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setContentsMargins(40,20,40,30);
    vlayout->addLayout(layout);
    vlayout->addStretch(120);
    vlayout->addLayout(layout1);

    q_func()->setMinimumWidth(400);
}

void ConfirmRestartFrmPrivate::InitData()
{
    q_func()->setObjectName("ConfirmRestart");
    
    m_pNameLabel->setText("Restart Imediately？");
    m_pNameLabel->setStyleSheet("color:#FE0000");

    m_pConfirmButton->setFixedHeight(50);
    m_pConfirmButton->setMinimumWidth(150);
    m_pConfirmButton->setContentsMargins(10,5,10,5);
    m_pCancelButton->setFixedHeight(50);
    m_pCancelButton->setMinimumWidth(150);
    m_pCancelButton->setContentsMargins(10,5,10,5);

    m_pConfirmButton->setText("Restart Now");
    m_pCancelButton->setText("Cancel");
}

void ConfirmRestartFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton,&QPushButton::clicked,[&]{q_func()->done(0);});
    QObject::connect(m_pCancelButton,&QPushButton::clicked,[&]{q_func()->done(1);});
}
