#include "FillLightFrm.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QStyle>

// Custom Option Card Widget
class OptionCardWidget : public QFrame
{
    Q_OBJECT
public:
    OptionCardWidget(const QString &text, int value, QWidget *parent = nullptr)
        : QFrame(parent), m_value(value), m_selected(false)
    {
        setObjectName("OptionCard");
        setupUI(text);
        setCursor(Qt::PointingHandCursor);
    }
    
    void setSelected(bool selected) {
        m_selected = selected;
        setProperty("selected", selected);
        style()->unpolish(this);
        style()->polish(this);
    }
    
    bool isSelected() const { return m_selected; }
    int getValue() const { return m_value; }

signals:
    void cardClicked(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        emit cardClicked(m_value);
    }

private:
    void setupUI(const QString &text) {
        setFixedHeight(60);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(20, 15, 20, 15);
        
        QLabel *textLabel = new QLabel(text);
        textLabel->setObjectName("OptionText");
        
        QLabel *radioIndicator = new QLabel();
        radioIndicator->setObjectName("RadioIndicator");
        radioIndicator->setFixedSize(18, 18);
        
        layout->addWidget(textLabel);
        layout->addStretch();
        layout->addWidget(radioIndicator);
    }

private:
    int m_value;
    bool m_selected;
};

class FillLightFrmPrivate
{
    Q_DECLARE_PUBLIC(FillLightFrm)
public:
    FillLightFrmPrivate(FillLightFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
    void SelectOption(int value);
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Option cards
    QList<OptionCardWidget*> m_optionCards;
    int m_selectedValue;
    
    // Action buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    FillLightFrm *const q_ptr;
};

FillLightFrmPrivate::FillLightFrmPrivate(FillLightFrm *dd)
    : q_ptr(dd), m_selectedValue(0)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

FillLightFrm::FillLightFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new FillLightFrmPrivate(this))
{

}

FillLightFrm::~FillLightFrm()
{

}

void FillLightFrmPrivate::InitUI()
{
    q_func()->setObjectName("FillLightFrm");
    q_func()->setFixedSize(500, 420);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_pTitleLabel = new QLabel(QObject::tr("Fill Light Setting"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(25, 25, 25, 25);
    contentLayout->setSpacing(25);
    
    // Options container
    QWidget *optionsWidget = new QWidget;
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsWidget);
    optionsLayout->setSpacing(15);
    
    // Create option cards
    OptionCardWidget *closedCard = new OptionCardWidget(QObject::tr("Normally Closed"), 0);
    OptionCardWidget *openCard = new OptionCardWidget(QObject::tr("Normally Open"), 1);
    OptionCardWidget *autoCard = new OptionCardWidget(QObject::tr("Auto"), 2);
    
    m_optionCards.append(closedCard);
    m_optionCards.append(openCard);
    m_optionCards.append(autoCard);
    
    optionsLayout->addWidget(closedCard);
    optionsLayout->addWidget(openCard);
    optionsLayout->addWidget(autoCard);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 20, 0, 0);
    actionsLayout->setSpacing(15);
    actionsLayout->setAlignment(Qt::AlignCenter);
    
    m_pCancelButton = new QPushButton(QObject::tr("Cancel"));
    m_pCancelButton->setObjectName("CancelButton");
    
    m_pConfirmButton = new QPushButton(QObject::tr("OK"));
    m_pConfirmButton->setObjectName("ConfirmButton");
    
    actionsLayout->addWidget(m_pCancelButton);
    actionsLayout->addWidget(m_pConfirmButton);
    
    // Add separator line
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("DialogSeparator");
    
    contentLayout->addWidget(optionsWidget);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void FillLightFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#FillLightFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
        "QWidget#DialogHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-top-left-radius: 15px;"
        "    border-top-right-radius: 15px;"
        "}"
        "QLabel#DialogTitle {"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        
        // Option card styles
        "OptionCardWidget#OptionCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 10px;"
        "}"
        "OptionCardWidget#OptionCard:hover {"
        "    border-color: #2196F3;"
        "    background: #f0f8ff;"
        "}"
        "OptionCardWidget#OptionCard[selected=\"true\"] {"
        "    border-color: #2196F3;"
        "    background: #e3f2fd;"
        "}"
        
        "QLabel#OptionText {"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    color: #2c3e50;"
        "}"
        "OptionCardWidget#OptionCard[selected=\"true\"] QLabel#OptionText {"
        "    color: #1976D2;"
        "}"
        
        "QLabel#RadioIndicator {"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 9px;"
        "    background: white;"
        "}"
        "OptionCardWidget#OptionCard[selected=\"true\"] QLabel#RadioIndicator {"
        "    border-color: #2196F3;"
        "    background: #2196F3;"
        "}"
        
        // Button styles
        "QPushButton#ConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#ConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#CancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#CancelButton:hover {"
        "    background: #545b62;"
        "}"
        
        // Separator
        "QFrame#DialogSeparator {"
        "    color: #e9ecef;"
        "    background-color: #e9ecef;"
        "}"
    );
}

void FillLightFrmPrivate::InitData()
{
    SelectOption(0); // Default to "Normally Closed"
}

void FillLightFrmPrivate::InitConnect()
{
    // Connect card selection
    for (OptionCardWidget *card : m_optionCards) {
        QObject::connect(card, &OptionCardWidget::cardClicked, 
                        [this](int value) {
                            SelectOption(value);
                        });
    }
    
    // Connect buttons
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [this] {
        q_func()->done(0);
    });
    
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [this] {
        q_func()->done(1);
    });
}

void FillLightFrmPrivate::SelectOption(int value)
{
    m_selectedValue = value;
    
    // Update card selection states
    for (OptionCardWidget *card : m_optionCards) {
        card->setSelected(card->getValue() == value);
    }
}

void FillLightFrm::setFillLightMode(const int &index)
{
    Q_D(FillLightFrm);
    d->SelectOption(index);
}

int FillLightFrm::getFillLightMode() const
{
    return d_func()->m_selectedValue;
}

void FillLightFrm::mousePressEvent(QMouseEvent *event)
{
    // Keep for backward compatibility, but cards handle clicks directly
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE
void FillLightFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "FillLightFrm.moc"