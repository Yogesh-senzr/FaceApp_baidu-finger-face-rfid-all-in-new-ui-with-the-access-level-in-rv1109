#include "CInputBaseDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

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
    void SetupCardStyle();
private:
    QFrame *m_pCardFrame;
    QLabel *m_pTitleLabel;
    QLineEdit *m_pInputEdit;
    QLabel *m_pPlaceholderLabel;
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    CInputBaseDialog *const q_ptr;
};

CInputBaseDialogPrivate::CInputBaseDialogPrivate(CInputBaseDialog *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->SetupCardStyle();
}

CInputBaseDialog::CInputBaseDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint)  // FIXED: Removed FramelessWindowHint
    , d_ptr(new CInputBaseDialogPrivate(this))
{
    // CRITICAL FIX: Prevent transparency
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_NoSystemBackground, false);
    
    // Set white background
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Background, Qt::white);
    setPalette(p);
}

CInputBaseDialog::~CInputBaseDialog()
{

}

void CInputBaseDialogPrivate::InitUI()
{
    // Create the main card frame
    m_pCardFrame = new QFrame(q_func());
    
    // CRITICAL FIX: Set white background on card frame
    m_pCardFrame->setAutoFillBackground(true);
    QPalette cardPalette = m_pCardFrame->palette();
    cardPalette.setColor(QPalette::Window, Qt::white);
    m_pCardFrame->setPalette(cardPalette);
    
    m_pTitleLabel = new QLabel(m_pCardFrame);
    m_pInputEdit = new QLineEdit(m_pCardFrame);
    m_pPlaceholderLabel = new QLabel(m_pCardFrame);

    m_pConfirmButton = new QPushButton(m_pCardFrame);
    m_pCancelButton = new QPushButton(m_pCardFrame);

    // Placeholder layout inside the input field
    QHBoxLayout *PlaceholderLayout = new QHBoxLayout;
    PlaceholderLayout->setContentsMargins(0, 0, 0, 0);
    PlaceholderLayout->addStretch();
    PlaceholderLayout->addWidget(m_pPlaceholderLabel);
    PlaceholderLayout->addSpacing(10);

    m_pInputEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pInputEdit->setLayout(PlaceholderLayout);

    // Title layout
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->addStretch();
    titleLayout->addWidget(m_pTitleLabel);
    titleLayout->addStretch();

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(15);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_pCancelButton);
    buttonLayout->addWidget(m_pConfirmButton);

    // Card content layout
    QVBoxLayout *cardLayout = new QVBoxLayout(m_pCardFrame);
    cardLayout->setContentsMargins(30, 25, 30, 25);
    cardLayout->setSpacing(20);
    cardLayout->addLayout(titleLayout);
    cardLayout->addWidget(m_pInputEdit);
    cardLayout->addLayout(buttonLayout);

    // Main dialog layout - centers the card
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->addStretch();
    mainLayout->addWidget(m_pCardFrame);
    mainLayout->addStretch();
}

void CInputBaseDialogPrivate::InitData()
{
    q_func()->setObjectName("InputBaseDialog");
    q_func()->setModal(false);
    
    m_pTitleLabel->setObjectName("DialogTitleLabel");
    m_pPlaceholderLabel->setStyleSheet("color: #999999; font-size: 14px;");

    m_pConfirmButton->setFixedHeight(32);
    m_pCancelButton->setFixedHeight(32);
    
    m_pConfirmButton->setFixedWidth(80);
    m_pCancelButton->setFixedWidth(80);

    m_pConfirmButton->setText(QObject::tr("Ok"));
    m_pCancelButton->setText(QObject::tr("Cancel"));
}

void CInputBaseDialogPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void CInputBaseDialogPrivate::SetupCardStyle()
{
    // Dialog background - SOLID WHITE
    q_func()->setStyleSheet(
        "QDialog#InputBaseDialog {"
        "    background-color: white;"  // FIXED: Changed from gray to white
        "}"
    );

    // Card frame styling
    m_pCardFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E0E0E0;"
        "}"
    );

    // Title styling
    m_pTitleLabel->setStyleSheet(
        "QLabel#DialogTitleLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #333333;"
        "    padding: 5px 0px;"
        "}"
    );

    // Input field styling
    m_pInputEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    padding: 12px 15px;"
        "    font-size: 14px;"
        "    color: #333333;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #007AFF;"
        "    background-color: white;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #CCCCCC;"
        "}"
    );

    // Button styling
    QString buttonStyle =
        "QPushButton {"
        "    border: none;"
        "    border-radius: 6px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    padding: 10px 20px;"
        "}";

    m_pConfirmButton->setStyleSheet(buttonStyle +
        "QPushButton {"
        "    background-color: #007AFF;"
        "    color: white;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056CC;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004499;"
        "}"
    );

    m_pCancelButton->setStyleSheet(buttonStyle +
        "QPushButton {"
        "    background-color: #F0F0F0;"
        "    color: #666666;"
        "}"
        "QPushButton:hover {"
        "    background-color: #E0E0E0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #D0D0D0;"
        "}"
    );

    // CRITICAL FIX: COMMENT OUT the shadow effect - this causes transparency!
    // QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(q_func());
    // shadowEffect->setBlurRadius(20);
    // shadowEffect->setColor(QColor(0, 0, 0, 60));
    // shadowEffect->setOffset(0, 4);
    // m_pCardFrame->setGraphicsEffect(shadowEffect);
    
    // Alternative: Use CSS box-shadow effect (optional, may not work well in Qt 5.9.4)
    // If you want shadow, use a manual drawing approach instead
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
    QRegExp regExp(text);
    QRegExpValidator *pattern= new QRegExpValidator(regExp, this);
    d->m_pInputEdit->setValidator(pattern);
}

QString CInputBaseDialog::getData() const
{
    return d_func()->m_pInputEdit->text();
}

void CInputBaseDialog::paintEvent(QPaintEvent *event)
{
    // CRITICAL FIX: Paint white background explicitly
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QDialog::paintEvent(event);
}