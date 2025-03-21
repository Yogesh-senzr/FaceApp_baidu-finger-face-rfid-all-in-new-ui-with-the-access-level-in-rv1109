#include "InputWifiPasswordFrm.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class InputWifiPasswordFrmPrivate
{
    Q_DECLARE_PUBLIC(InputWifiPasswordFrm)
public:
    InputWifiPasswordFrmPrivate(InputWifiPasswordFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;
    QLineEdit *m_pPasswordEdit;//密码
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    QString mService;
private:
    InputWifiPasswordFrm *const q_ptr;
};

InputWifiPasswordFrmPrivate::InputWifiPasswordFrmPrivate(InputWifiPasswordFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

InputWifiPasswordFrm::InputWifiPasswordFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new InputWifiPasswordFrmPrivate(this))
{

}

InputWifiPasswordFrm::~InputWifiPasswordFrm()
{

}

void InputWifiPasswordFrmPrivate::InitUI()
{
    m_pNameLabel = new QLabel;//wifi名称
    m_pPasswordEdit = new QLineEdit;//密码
    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_pNameLabel);
    layout->addStretch();

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_pConfirmButton);
    layout1->addWidget(m_pCancelButton);

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(10, 5, 10, 10);
    vLayout->addLayout(layout);
    vLayout->addStretch();
    vLayout->addWidget(m_pPasswordEdit);
    vLayout->addStretch();
    vLayout->addLayout(layout1);
}

void InputWifiPasswordFrmPrivate::InitData()
{
    q_func()->setObjectName("InputWifiPassword");

    m_pPasswordEdit->setPlaceholderText(QObject::tr("Password"));//密码

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void InputWifiPasswordFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void InputWifiPasswordFrm::setData(const QString &Service, const QString &Name)
{
    Q_D(InputWifiPasswordFrm);
    d->m_pNameLabel->setText(Name);
    d->mService = Service;
}

QString InputWifiPasswordFrm::getPassword()
{
    return d_func()->m_pPasswordEdit->text();
}

QString InputWifiPasswordFrm::getWifiName() const
{
    return d_func()->m_pNameLabel->text();
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void InputWifiPasswordFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif