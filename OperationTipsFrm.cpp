#include "OperationTipsFrm.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class OperationTipsFrmPrivate
{
    Q_DECLARE_PUBLIC(OperationTipsFrm)
public:
    OperationTipsFrmPrivate(OperationTipsFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pLeftBtn;
    QPushButton *m_pRightBtn;
    QLabel *m_pTitleLabel;
    QLabel *m_pTooltipTextLabel;
private:
    OperationTipsFrm *const q_ptr;
};

OperationTipsFrmPrivate::OperationTipsFrmPrivate(OperationTipsFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

OperationTipsFrm::OperationTipsFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowCloseButtonHint)
    , d_ptr(new OperationTipsFrmPrivate(this))
{
}

OperationTipsFrm::~OperationTipsFrm()
{
}

void OperationTipsFrmPrivate::InitUI()
{
    m_pLeftBtn = new QPushButton(QObject::tr("Ok"));//确定
    m_pRightBtn = new QPushButton(QObject::tr("Cancel"));//取消
    m_pTitleLabel = new QLabel(QObject::tr("Tips"));//温馨提示
    m_pTooltipTextLabel = new QLabel("");

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->addStretch();
    titleLayout->addWidget(m_pTitleLabel);
    titleLayout->addStretch();

    QHBoxLayout *tooptipLayout = new QHBoxLayout;
    tooptipLayout->addSpacing(10);
    tooptipLayout->addWidget(m_pTooltipTextLabel);
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
    makeLayout->addStretch();
    makeLayout->addLayout(tooptipLayout);
    makeLayout->addStretch();
    makeLayout->addLayout(BtnLayout);
}

void OperationTipsFrmPrivate::InitData()
{
    q_func()->setObjectName("OperationTipsFrm");

    int width = QApplication::desktop()->screenGeometry().width();
    if(width <= 480){
        q_func()->setFixedSize(440,280);
    }else{
        q_func()->setFixedSize(540,280);
    }
        
    m_pLeftBtn->setFixedHeight(64);
    m_pRightBtn->setFixedHeight(64);
    m_pTooltipTextLabel->setWordWrap(true);
    m_pTooltipTextLabel->setAlignment(Qt::AlignCenter);

    m_pTitleLabel->setObjectName("DialogTitleLabel");
    m_pTooltipTextLabel->setObjectName("DialogLabel");
}

void OperationTipsFrmPrivate::InitConnect()
{
    QObject::connect(m_pLeftBtn, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pRightBtn, &QPushButton::clicked, [&] {q_func()->done(1); });
}

int OperationTipsFrm::setMessageBox(const QString &title, const QString &text, const QString &confirm, const QString &cancel, const int btnshow)
{
    Q_D(OperationTipsFrm);
    d->m_pTitleLabel->setText(title);
    d->m_pTooltipTextLabel->setText(text);
    d->m_pLeftBtn->setText(confirm);
    d->m_pRightBtn->setText(cancel);

    if (btnshow == 0)
    {
        d->m_pLeftBtn->setVisible(false);
        d->m_pRightBtn->setVisible(false);
    }
    else if (btnshow == 1)
    {
        d->m_pLeftBtn->setVisible(true);
        d->m_pRightBtn->setVisible(false);
    }
    else if (btnshow == 2)
    {
        d->m_pLeftBtn->setVisible(false);
        d->m_pRightBtn->setVisible(true);
    }
    else
    {
        d->m_pLeftBtn->setVisible(true);
        d->m_pRightBtn->setVisible(true);
    }
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");        
#endif     
    return exec();
}
