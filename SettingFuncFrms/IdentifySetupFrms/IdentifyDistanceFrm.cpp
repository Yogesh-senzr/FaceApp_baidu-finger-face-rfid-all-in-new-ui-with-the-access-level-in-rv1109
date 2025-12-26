#include "IdentifyDistanceFrm.h"

#include "Config/ReadConfig.h"

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>
// Custom Distance Card Widget
class DistanceCardWidget : public QFrame
{
    Q_OBJECT
public:
    DistanceCardWidget(const QString &distance, int value, QWidget *parent = nullptr)
        : QFrame(parent), m_value(value), m_selected(false)
    {
        setObjectName("DistanceCard");
        setupUI(distance);
        setCursor(Qt::PointingHandCursor);
    }
    
    void setSelected(bool selected) {
        m_selected = selected;
        updateSelectedState();
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
    
    void enterEvent(QEvent *event) override {
        Q_UNUSED(event);
        if (!m_selected) {
            setProperty("hovered", true);
            style()->unpolish(this);
            style()->polish(this);
        }
    }
    
    void leaveEvent(QEvent *event) override {
        Q_UNUSED(event);
        setProperty("hovered", false);
        style()->unpolish(this);
        style()->polish(this);
    }

private:
    void setupUI(const QString &distance) {
        setFixedHeight(90); // Increased height for better spacing
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(25, 20, 25, 20); // Increased margins
        
        // Distance value label
        m_distanceLabel = new QLabel(distance);
        m_distanceLabel->setObjectName("DistanceValue");
        
        // Radio indicator
        m_radioIndicator = new QLabel();
        m_radioIndicator->setObjectName("RadioIndicator");
        m_radioIndicator->setFixedSize(24, 24); // Slightly larger indicator
        
        layout->addWidget(m_distanceLabel);
        layout->addStretch();
        layout->addWidget(m_radioIndicator);
    }
    
    void updateSelectedState() {
        setProperty("selected", m_selected);
        style()->unpolish(this);
        style()->polish(this);
    }

private:
    int m_value;
    bool m_selected;
    QLabel *m_distanceLabel;
    QLabel *m_radioIndicator;
};

class IdentifyDistanceFrmPrivate
{
    Q_DECLARE_PUBLIC(IdentifyDistanceFrm)
public:
    IdentifyDistanceFrmPrivate(IdentifyDistanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
    void SelectCard(int value);
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    QLabel *m_pSubtitleLabel;
    
    // Distance cards
    QList<DistanceCardWidget*> m_distanceCards;
    int m_selectedValue;
    
    // Action buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    IdentifyDistanceFrm *const q_ptr;
};

IdentifyDistanceFrmPrivate::IdentifyDistanceFrmPrivate(IdentifyDistanceFrm *dd)
    : q_ptr(dd), m_selectedValue(1) // Default to 100cm
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

IdentifyDistanceFrm::IdentifyDistanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new IdentifyDistanceFrmPrivate(this))
{

}

IdentifyDistanceFrm::~IdentifyDistanceFrm()
{

}

void IdentifyDistanceFrmPrivate::InitUI()
{
    q_func()->setObjectName("IdentifyDistanceFrm");
    q_func()->setFixedSize(600, 550); // Significantly increased size: width 500->600, height 400->550
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(120); // Increased header height from 100 to 120
    
    QVBoxLayout *headerLayout = new QVBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(35, 30, 35, 30); // Increased margins
    headerLayout->setSpacing(8); // Increased spacing
    headerLayout->setAlignment(Qt::AlignCenter);
    
    m_pTitleLabel = new QLabel(QObject::tr("Recognition Distance"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    m_pSubtitleLabel = new QLabel(QObject::tr("Select the optimal distance for face recognition"));
    m_pSubtitleLabel->setObjectName("DialogSubtitle");
    m_pSubtitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    headerLayout->addWidget(m_pSubtitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 40, 40, 40); // Increased margins from 30 to 40
    contentLayout->setSpacing(25); // Increased spacing from 20 to 25
    
    // Distance cards
    QWidget *cardsWidget = new QWidget;
    QVBoxLayout *cardsLayout = new QVBoxLayout(cardsWidget);
    cardsLayout->setSpacing(25); // Increased spacing between cards from 20 to 25
    
    // Create distance cards
    DistanceCardWidget *card50 = new DistanceCardWidget(QObject::tr("50cm"), 0);
    DistanceCardWidget *card100 = new DistanceCardWidget(QObject::tr("100cm"), 1);
    DistanceCardWidget *card150 = new DistanceCardWidget(QObject::tr("150cm"), 2);
    
    m_distanceCards.append(card50);
    m_distanceCards.append(card100);
    m_distanceCards.append(card150);
    
    cardsLayout->addWidget(card50);
    cardsLayout->addWidget(card100);
    cardsLayout->addWidget(card150);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 25, 0, 0); // Increased top margin from 20 to 25
    actionsLayout->setSpacing(20); // Increased spacing between buttons from 15 to 20
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
    
    contentLayout->addWidget(cardsWidget);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void IdentifyDistanceFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#IdentifyDistanceFrm {"
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
        "    font-size: 26px;" // Slightly increased font size
        "    font-weight: 600;"
        "}"
        "QLabel#DialogSubtitle {"
        "    color: white;"
        "    font-size: 15px;" // Slightly increased font size
        "    opacity: 0.9;"
        "}"
        
        // Distance card styles
        "DistanceCardWidget#DistanceCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 12px;"
        "}"
        "DistanceCardWidget#DistanceCard:hover {"
        "    border-color: #2196F3;"
        "    background: #f0f8ff;"
        "}"
        "DistanceCardWidget#DistanceCard[selected=\"true\"] {"
        "    border-color: #2196F3;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #e3f2fd, stop:1 #f0f8ff);"
        "}"
        
        "QLabel#DistanceValue {"
        "    font-size: 32px;" // Increased font size from 28px to 32px
        "    font-weight: 700;"
        "    color: #2c3e50;"
        "}"
        "DistanceCardWidget#DistanceCard[selected=\"true\"] QLabel#DistanceValue {"
        "    color: #1976D2;"
        "}"
        
        "QLabel#RadioIndicator {"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 12px;" // Increased border radius from 10px to 12px
        "    background: white;"
        "}"
        "DistanceCardWidget#DistanceCard[selected=\"true\"] QLabel#RadioIndicator {"
        "    border-color: #2196F3;"
        "    background: #2196F3;"
        "}"
        
        // Button styles
        "QPushButton#ConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 16px 36px;" // Increased padding
        "    border-radius: 8px;"
        "    font-size: 17px;" // Slightly increased font size
        "    font-weight: 600;"
        "    min-width: 130px;" // Increased minimum width
        "}"
        "QPushButton#ConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#CancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 16px 36px;" // Increased padding
        "    border-radius: 8px;"
        "    font-size: 17px;" // Slightly increased font size
        "    font-weight: 600;"
        "    min-width: 130px;" // Increased minimum width
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

void IdentifyDistanceFrmPrivate::InitData()
{
    // Set default selection based on config
    int index = ReadConfig::GetInstance()->getIdentity_Manager_Identifycm();
    SelectCard(index);
}

void IdentifyDistanceFrmPrivate::InitConnect()
{
    // Connect card selection
    for (DistanceCardWidget *card : m_distanceCards) {
        QObject::connect(card, &DistanceCardWidget::cardClicked, 
                        [this](int value) {
                            SelectCard(value);
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

void IdentifyDistanceFrmPrivate::SelectCard(int value)
{
    m_selectedValue = value;
    
    // Update card selection states
    for (DistanceCardWidget *card : m_distanceCards) {
        card->setSelected(card->getValue() == value);
    }
}

void IdentifyDistanceFrm::setDistanceMode(const int &index)
{
    Q_D(IdentifyDistanceFrm);
    d->SelectCard(index);
}

int IdentifyDistanceFrm::getDistanceMode() const
{
    return d_func()->m_selectedValue;
}

void IdentifyDistanceFrm::mousePressEvent(QMouseEvent *event)
{
    // Keep your existing mouse press handling for backward compatibility
    // but the card widgets will handle clicks more elegantly
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE
void IdentifyDistanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif

#include "IdentifyDistanceFrm.moc"