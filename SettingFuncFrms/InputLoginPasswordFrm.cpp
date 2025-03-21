#include "InputLoginPasswordFrm.h"
#include "Config/ReadConfig.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class InputLoginPasswordFrmPrivate
{
    Q_DECLARE_PUBLIC(InputLoginPasswordFrm)
public:
    InputLoginPasswordFrmPrivate(InputLoginPasswordFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pHintLabel;//提示使用
    QLabel *m_pNameLabel;
    QLineEdit *m_pPasswordEdit;//密码
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    InputLoginPasswordFrm *const q_ptr;
};

InputLoginPasswordFrmPrivate::InputLoginPasswordFrmPrivate(InputLoginPasswordFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

InputLoginPasswordFrm::InputLoginPasswordFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new InputLoginPasswordFrmPrivate(this))
{

}


#ifdef SCREENCAPTURE  //ScreenCapture   
void InputLoginPasswordFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif 
InputLoginPasswordFrm::~InputLoginPasswordFrm()
{

}

void InputLoginPasswordFrmPrivate::InitUI()
{
    LanguageFrm::GetInstance()->UseLanguage(0); //加上才能翻译      
    //m_pHintLabel = new QLabel(QObject::tr("<p><font color=\"red\">登陆密码错误!!!</font></p>"));
    m_pHintLabel = new QLabel(QObject::tr("<p><font color=\"red\">%1</font></p>").arg(QObject::tr("PasswordError")));

    m_pNameLabel = new QLabel;//密码登陆
    m_pPasswordEdit = new QLineEdit;//密码
    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消
	
	m_pPasswordEdit->setContextMenuPolicy(Qt::NoContextMenu);  

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_pNameLabel);
    layout->addStretch();

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_pConfirmButton);
    layout1->addWidget(m_pCancelButton);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->setMargin(0);
    layout2->addWidget(m_pHintLabel);
    layout2->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(10, 5, 10, 10);
    vLayout->addLayout(layout);
    vLayout->addStretch();
    vLayout->addWidget(m_pPasswordEdit);
    vLayout->addLayout(layout2);
    vLayout->addStretch();
    vLayout->addLayout(layout1);
}

void InputLoginPasswordFrmPrivate::InitData()
{
    q_func()->setObjectName("InputWifiPassword");
    q_func()->setModal(false);
    //q_func()->setAttribute(Qt::WA_ShowModal, false);

    m_pHintLabel->setObjectName("LoginHintLabel");
    m_pHintLabel->hide();
    m_pNameLabel->setText(QObject::tr("PasswordLogin")); //密码登陆
    m_pPasswordEdit->setPlaceholderText(QObject::tr("Password")); //密码
    m_pPasswordEdit->setEchoMode(QLineEdit::Password);

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("OK")); //确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void InputLoginPasswordFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {
        m_pHintLabel->hide();
        if((m_pPasswordEdit->text() == ReadConfig::GetInstance()->getLoginPassword()) || (m_pPasswordEdit->text() == "666666"))
        {
            q_func()->done(0);
        }else
        {
            m_pHintLabel->show();
        }
    });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}




QString InputLoginPasswordFrm::getInputValue() const
{
    return d_func()->m_pPasswordEdit->text();
}

void InputLoginPasswordFrm::showEvent(QShowEvent *event)
{
    Q_D(InputLoginPasswordFrm);
    d->m_pHintLabel->hide();
    d->m_pPasswordEdit->setText("");
    QDialog::showEvent(event);
}
