#include "CInputBaseDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

#include <QPainter>
#include <QStyleOption>
#include <QIntValidator>
#include <QDoubleValidator>

class CInputBaseDialogPrivate
{
    Q_DECLARE_PUBLIC(CInputBaseDialog)
public:
    CInputBaseDialogPrivate(CInputBaseDialog *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pTitleLabel;
    QLineEdit *m_pInputEdit;
    QLabel *m_pPlaceholderLabel;//提示Label
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    CInputBaseDialog *const q_ptr;
};


CInputBaseDialogPrivate::CInputBaseDialogPrivate(CInputBaseDialog *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CInputBaseDialog::CInputBaseDialog(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new CInputBaseDialogPrivate(this))
{

}

CInputBaseDialog::~CInputBaseDialog()
{

}

void CInputBaseDialogPrivate::InitUI()
{
    m_pTitleLabel = new QLabel;
    m_pInputEdit = new QLineEdit;
    m_pPlaceholderLabel = new QLabel;

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *PlaceholderLayout = new QHBoxLayout;
    PlaceholderLayout->setMargin(0);
    PlaceholderLayout->addStretch();
    PlaceholderLayout->addWidget(m_pPlaceholderLabel);
    PlaceholderLayout->addSpacing(10);

    m_pInputEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pInputEdit->setLayout(PlaceholderLayout);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_pTitleLabel);
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_pConfirmButton);
    layout1->addWidget(m_pCancelButton);

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(10, 5, 10, 10);
    vLayout->addLayout(layout);
    vLayout->addStretch();
    vLayout->addWidget(m_pInputEdit);
    vLayout->addStretch();
    vLayout->addLayout(layout1);
}

void CInputBaseDialogPrivate::InitData()
{
    q_func()->setObjectName("InputBaseDialog");
    q_func()->setModal(false);
    m_pTitleLabel->setObjectName("DialogTitleLabel");
    m_pPlaceholderLabel->setStyleSheet("color:gray;font-size:26px;");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void CInputBaseDialogPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void CInputBaseDialog::setTitleText(const QString &text)
{
    Q_D(CInputBaseDialog);
    d->m_pTitleLabel->setText(text);
}

void CInputBaseDialog::setPlaceholderText(const QString &text)
{
    Q_D(CInputBaseDialog);
    d->m_pPlaceholderLabel->setText(text);
}

void CInputBaseDialog::setData(const QString &text)
{
    Q_D(CInputBaseDialog);
    d->m_pInputEdit->setText(text);
}

void CInputBaseDialog::setIntValidator(const int &min, const int &max)
{
    Q_D(CInputBaseDialog);
    d->m_pInputEdit->setValidator(new QIntValidator(min, max, this));
}

void CInputBaseDialog::setFloatValidator(const double &min, const double &max)
{
    Q_D(CInputBaseDialog);
    d->m_pInputEdit->setValidator(new QDoubleValidator(min, max, 2, this));
}

void CInputBaseDialog::setRegExpValidator(const QString &text)
{
    Q_D(CInputBaseDialog);
    QRegExp regExp(text);//创建了一个模式
    QRegExpValidator *pattern= new QRegExpValidator(regExp, this);//创建了一个表达式
    d->m_pInputEdit->setValidator(pattern);//交付使用
}

QString CInputBaseDialog::getData() const
{
    return d_func()->m_pInputEdit->text();
}

void CInputBaseDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QDialog::paintEvent(event);
}
