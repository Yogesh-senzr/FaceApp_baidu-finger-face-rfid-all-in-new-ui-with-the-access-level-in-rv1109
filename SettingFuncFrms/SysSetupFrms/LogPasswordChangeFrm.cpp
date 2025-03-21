#include "LogPasswordChangeFrm.h"
#include "Config/ReadConfig.h"

#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>

class LogPasswordChangeFrmPrivate
{
    Q_DECLARE_PUBLIC(LogPasswordChangeFrm)
public:
    LogPasswordChangeFrmPrivate(LogPasswordChangeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pHintLabel;//提示使用
    QLineEdit *m_pOldPassword;
    QLineEdit *m_pNewPassword;
    QLineEdit *m_pConfirmNewPassword;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    LogPasswordChangeFrm *const q_ptr;
};

LogPasswordChangeFrmPrivate::LogPasswordChangeFrmPrivate(LogPasswordChangeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

LogPasswordChangeFrm::LogPasswordChangeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LogPasswordChangeFrmPrivate(this))
{

}

LogPasswordChangeFrm::~LogPasswordChangeFrm()
{

}

void LogPasswordChangeFrmPrivate::InitUI()
{
    m_pHintLabel = new QLabel;

    m_pOldPassword = new QLineEdit;
    m_pNewPassword = new QLineEdit;
    m_pConfirmNewPassword = new QLineEdit;
	
	m_pOldPassword->setContextMenuPolicy(Qt::NoContextMenu);
	m_pNewPassword->setContextMenuPolicy(Qt::NoContextMenu);
	m_pConfirmNewPassword->setContextMenuPolicy(Qt::NoContextMenu);

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("LoginPasswordReset")));//登陆密码修改
    layout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_pOldPassword);
    vLayout->addWidget(m_pNewPassword);
    vLayout->addWidget(m_pConfirmNewPassword);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addLayout(vLayout);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(m_pConfirmButton);
    layout2->addWidget(m_pCancelButton);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->setMargin(0);
    layout3->addWidget(m_pHintLabel);
    layout3->addStretch();

    QVBoxLayout *vLayout1 = new QVBoxLayout(q_func());
    vLayout1->setMargin(10);
    vLayout1->addLayout(layout);
    vLayout1->addLayout(layout1);
    vLayout1->addLayout(layout3);
    vLayout1->addLayout(layout2);
}

void LogPasswordChangeFrmPrivate::InitData()
{
    q_func()->setFixedSize(520, 370);

    m_pHintLabel->setObjectName("LoginHintLabel");
    m_pHintLabel->hide();

    m_pOldPassword->setPlaceholderText(QObject::tr("InputOldPassword"));//请输入旧密码
    m_pNewPassword->setPlaceholderText(QObject::tr("InputNewPassword"));//请输入新密码
    m_pConfirmNewPassword->setPlaceholderText(QObject::tr("ConfirmNewPasswordd"));//确认新密码

    m_pOldPassword->setFixedHeight(52);
    m_pNewPassword->setFixedHeight(52);
    m_pConfirmNewPassword->setFixedHeight(52);

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void LogPasswordChangeFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {
        m_pHintLabel->hide();
        if(m_pNewPassword->text() == m_pConfirmNewPassword->text())
        {
            if((m_pOldPassword->text() == ReadConfig::GetInstance()->getLoginPassword()))
            {
                q_func()->done(0);
            }else
            {
                //m_pHintLabel->setText(QObject::tr("<p><font color=\"red\"></font></p>"));
                m_pHintLabel->setText(QObject::tr("<p><font color=\"red\">%1</font></p>").arg(QObject::tr("WrongOldPassword")));//旧密码错误!!! Wrong old password,
                m_pHintLabel->show();
            }
        }else
        {
            m_pHintLabel->setText(QObject::tr("<p><font color=\"red\">%1</font></p>").arg(QObject::tr("PasswordNotMatch")));//俩次输入密码不一致!!!
            m_pHintLabel->show();
        }
    });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

QString LogPasswordChangeFrm::getOldPasw()
{
    return d_func()->m_pOldPassword->text();
}

QString LogPasswordChangeFrm::getNewPasw()
{
    return d_func()->m_pNewPassword->text();
}

QString LogPasswordChangeFrm::getConfirmNewPasw()
{
    return d_func()->m_pConfirmNewPassword->text();
}

void LogPasswordChangeFrm::showEvent(QShowEvent *event)
{
    Q_D(LogPasswordChangeFrm);
    d->m_pHintLabel->hide();
    d->m_pNewPassword->clear();
    d->m_pConfirmNewPassword->clear();
    d->m_pOldPassword->clear();
    QDialog::showEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void LogPasswordChangeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 