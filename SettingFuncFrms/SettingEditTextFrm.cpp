#include "SettingEditTextFrm.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QApplication>
#include <QDesktopWidget>


class SettingEditTextFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingEditTextFrm)
public:
    SettingEditTextFrmPrivate(SettingEditTextFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pLeftBtn;
    QPushButton *m_pRightBtn;
    QLabel *m_pTitleLabel;
    QLabel *m_pTooltipTextLabel;
    QTextEdit *m_pTextEdit;
private:
    SettingEditTextFrm *const q_ptr;
};

SettingEditTextFrmPrivate::SettingEditTextFrmPrivate(SettingEditTextFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SettingEditTextFrm::SettingEditTextFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowCloseButtonHint)
    , d_ptr(new SettingEditTextFrmPrivate(this))    
{

}

SettingEditTextFrm::~SettingEditTextFrm()
{

}

void SettingEditTextFrmPrivate::InitUI()
{
    m_pLeftBtn = new QPushButton(QObject::tr("Ok"));//确定
    m_pRightBtn = new QPushButton(QObject::tr("Cancel"));//取消
    m_pTitleLabel = new QLabel(QObject::tr("Tips"));//温馨提示
    m_pTooltipTextLabel = new QLabel("");
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setFont(QFont("宋体", 16-8));
    m_pTextEdit->setContextMenuPolicy(Qt::NoContextMenu); 
    m_pTextEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pTextEdit->setReadOnly(true);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->addStretch();
    titleLayout->addWidget(m_pTitleLabel);
    titleLayout->addStretch();

    QHBoxLayout *tooptipLayout = new QHBoxLayout;
    tooptipLayout->addSpacing(10);
    //tooptipLayout->addWidget(m_pTooltipTextLabel);
    tooptipLayout->addWidget(m_pTextEdit);
    tooptipLayout->addSpacing(10);

    QHBoxLayout *BtnLayout = new QHBoxLayout;
    BtnLayout->addSpacing(10);
    BtnLayout->addWidget(m_pLeftBtn);
    BtnLayout->addWidget(m_pRightBtn);
    BtnLayout->addSpacing(10);

    QVBoxLayout *makeLayout = new QVBoxLayout(q_func());
    makeLayout->setContentsMargins(0, 5, 0, 15);
    makeLayout->addLayout(titleLayout);
    makeLayout->addSpacing(5);
    makeLayout->addLayout(tooptipLayout,20);
    makeLayout->addStretch();
    makeLayout->addLayout(BtnLayout);
}

void SettingEditTextFrmPrivate::InitData()
{
    q_func()->setObjectName("SettingEditTextFrm");
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
		q_func()->setFixedSize(420,760);
	else 
      q_func()->setFixedSize(640,800-200+160);
    q_func()->move(80,0+80);

    m_pLeftBtn->setFixedHeight(64);
    m_pRightBtn->setFixedHeight(64);
    m_pTooltipTextLabel->setWordWrap(true);
    m_pTooltipTextLabel->setAlignment(Qt::AlignCenter);

    m_pTitleLabel->setObjectName("DialogTitleLabel");
    m_pTooltipTextLabel->setObjectName("DialogLabel");
}

void SettingEditTextFrmPrivate::InitConnect()
{
    QObject::connect(m_pLeftBtn, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pRightBtn, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void SettingEditTextFrm::setTitle(const QString &Name)
{
    Q_D(SettingEditTextFrm);
    //d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    d->m_pTitleLabel->setText(Name);
    //d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/ListWidgetTmpRight.png"));
}
void SettingEditTextFrm::setData(const QString &Name)
{
    Q_D(SettingEditTextFrm);
    //d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    d->m_pTextEdit->setText(Name);
    //d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/ListWidgetTmpRight.png"));
}

#ifdef SCREENCAPTURE  //ScreenCapture   
void SettingEditTextFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif