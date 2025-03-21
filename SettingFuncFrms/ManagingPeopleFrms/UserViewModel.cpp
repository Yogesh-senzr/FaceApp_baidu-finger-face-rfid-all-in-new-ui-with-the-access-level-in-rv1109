#include "UserViewModel.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class UserViewModelPrivate
{
    Q_DECLARE_PUBLIC(UserViewModel)
public:
    UserViewModelPrivate(UserViewModel *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;//姓名
    QLabel *m_pSexLabel;//性别
    QLabel *m_picCardLabel;//卡号
    QLabel *m_pidCardLabel;//身份证
    QLabel *m_pAddTimeLabel;//添加时间
    QPushButton *m_pDeleteButton;//删除
private:
    UserViewModel *const q_ptr;
};

UserViewModelPrivate::UserViewModelPrivate(UserViewModel *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

UserViewModel::UserViewModel(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new UserViewModelPrivate(this))
{

}

UserViewModel::~UserViewModel()
{

}

void UserViewModelPrivate::InitUI()
{
    m_pNameLabel = new QLabel;//姓名
    m_pSexLabel = new QLabel;//性别
    m_picCardLabel = new QLabel;//卡号
    m_pidCardLabel = new QLabel;//身份证
    m_pAddTimeLabel = new QLabel;//添加时间
    m_pDeleteButton = new QPushButton;//删除

    QFrame *f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("UserViewModelFrm");
    f->setFixedWidth(1);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_pNameLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("UserViewModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pSexLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("UserViewModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_picCardLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("UserViewModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pidCardLabel);
    layout->addWidget(f);
    layout->addStretch();

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("UserViewModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pAddTimeLabel);
    layout->addWidget(f);

    layout->addSpacing(20);
    layout->addWidget(m_pDeleteButton);

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->setSpacing(0);
    malayout->addLayout(layout);
}

void UserViewModelPrivate::InitData()
{
    m_pAddTimeLabel->setWordWrap(true);
    m_pNameLabel->setAlignment(Qt::AlignCenter);
    m_pSexLabel->setAlignment(Qt::AlignCenter);
    m_picCardLabel->setAlignment(Qt::AlignCenter);
    m_pidCardLabel->setAlignment(Qt::AlignCenter);
    m_pAddTimeLabel->setAlignment(Qt::AlignCenter);

    m_pNameLabel->setFixedWidth(132);
    m_pSexLabel->setFixedWidth(100);
    m_picCardLabel->setFixedWidth(130);
    m_pidCardLabel->setFixedWidth(260);
    m_pAddTimeLabel->setFixedWidth(140);

    m_pAddTimeLabel->setFixedHeight(80);

    m_pDeleteButton->setFixedSize(100, 52);
    m_pDeleteButton->setText(QObject::tr("Delete"));//"删除"
    m_pDeleteButton->hide();
    //    m_pNameLabel->setStyleSheet("background:red");
    //    m_pSexLabel->setStyleSheet("background:green");
    //    m_pTemperatureLabel->setStyleSheet("background:blue");
    //    m_pTransitTimeLabel->setStyleSheet("background:yellow");
    //    m_pHumanFaceLabel->setStyleSheet("background:black");

    m_pNameLabel->setStyleSheet("QLabel{font-size:24px;}");
    m_pSexLabel->setStyleSheet("QLabel{font-size:24px;}");
    m_picCardLabel->setStyleSheet("QLabel{font-size:24px;}");
    m_pidCardLabel->setStyleSheet("QLabel{font-size:24px;}");
    m_pAddTimeLabel->setStyleSheet("QLabel{font-size:24px;}");
}

void UserViewModelPrivate::InitConnect()
{
    QObject::connect(m_pDeleteButton, &QPushButton::clicked, q_func(), &UserViewModel::sigDeleteButton);
}

void UserViewModel::setData(const QString &name, const QString &sex, const QString &icCard, const QString &idCard, const QString &addtime)
{
    Q_D(UserViewModel);
    d->m_pNameLabel->setText(name);
    d->m_pSexLabel->setText(sex);
    d->m_picCardLabel->setText(icCard);
    d->m_pidCardLabel->setText(idCard);
    d->m_pAddTimeLabel->setText(addtime);

    if(name.isEmpty())d->m_pDeleteButton->hide();
    else d->m_pDeleteButton->show();
}

QString UserViewModel::getName() const
{
    return d_func()->m_pNameLabel->text();
}

QString UserViewModel::getCreateTime() const
{
    return d_func()->m_pAddTimeLabel->text();
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void UserViewModel::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 