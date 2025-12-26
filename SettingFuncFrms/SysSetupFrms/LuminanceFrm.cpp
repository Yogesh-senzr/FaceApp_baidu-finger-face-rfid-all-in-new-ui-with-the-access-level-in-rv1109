#include "LuminanceFrm.h"
#include "sliderclick.h"
#ifdef Q_OS_LINUX
#include "PCIcore/Display.h"
#endif

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

class LuminanceFrmPrivate
{
    Q_DECLARE_PUBLIC(LuminanceFrm)
public:
    LuminanceFrmPrivate(LuminanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyModernStyles();
    void CreateHeader();
    void CreateBrightnessCard();
    void UpdateBrightnessDisplay(int value);
    QString getBrightnessDescription(int value);
private:
    // Header components
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Main content
    QWidget *m_pContentWidget;
    QFrame *m_pBrightnessCard;
    
    // Brightness display
    QWidget *m_pDisplayWidget;
    QLabel *m_pBrightnessIcon;
    QLabel *m_pBrightnessValue;
    QLabel *m_pBrightnessLabel;
    
    // Slider section
    QWidget *m_pSliderWidget;
    QLabel *m_pLeftHintLabel;
    SliderClick *m_pAdjustSlider;
    QLabel *m_pRightHintLabel;

    
private:
    LuminanceFrm *const q_ptr;
};

LuminanceFrmPrivate::LuminanceFrmPrivate(LuminanceFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyModernStyles();
}

LuminanceFrm::LuminanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LuminanceFrmPrivate(this))
{
    setFixedSize(450, 350);
    
    // Center the dialog
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
}

LuminanceFrm::~LuminanceFrm()
{

}

void LuminanceFrmPrivate::InitUI()
{
    q_func()->setObjectName("LuminanceFrm");
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    CreateHeader();
    CreateBrightnessCard();
    
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(m_pContentWidget, 1);
}

void LuminanceFrmPrivate::CreateHeader()
{
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(80);
    
    QVBoxLayout *headerLayout = new QVBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(30, 25, 30, 25);
    
    m_pTitleLabel = new QLabel(QObject::tr("BrightnessSetting"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
}

void LuminanceFrmPrivate::CreateBrightnessCard()
{
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("ContentWidget");
    
    QVBoxLayout *contentLayout = new QVBoxLayout(m_pContentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    
    // Create brightness card
    m_pBrightnessCard = new QFrame;
    m_pBrightnessCard->setObjectName("BrightnessCard");
    
    QVBoxLayout *cardLayout = new QVBoxLayout(m_pBrightnessCard);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(25);
    
    // Brightness display section
    m_pDisplayWidget = new QWidget;
    QVBoxLayout *displayLayout = new QVBoxLayout(m_pDisplayWidget);
    displayLayout->setContentsMargins(0, 0, 0, 0);
    displayLayout->setSpacing(10);
    displayLayout->setAlignment(Qt::AlignCenter);
    
    m_pBrightnessIcon = new QLabel(" ☀ "); // Sun icon
    m_pBrightnessIcon->setObjectName("BrightnessIcon");
    m_pBrightnessIcon->setAlignment(Qt::AlignCenter);
    
    m_pBrightnessValue = new QLabel("75");
    m_pBrightnessValue->setObjectName("BrightnessValue");
    m_pBrightnessValue->setAlignment(Qt::AlignCenter);
    
    m_pBrightnessLabel = new QLabel(QObject::tr("BRIGHTNESS LEVEL"));
    m_pBrightnessLabel->setObjectName("BrightnessLabel");
    m_pBrightnessLabel->setAlignment(Qt::AlignCenter);
    
    displayLayout->addWidget(m_pBrightnessIcon);
    displayLayout->addWidget(m_pBrightnessValue);
    displayLayout->addWidget(m_pBrightnessLabel);
    
    // Slider section
    m_pSliderWidget = new QWidget;
    QVBoxLayout *sliderLayout = new QVBoxLayout(m_pSliderWidget);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(15);
    
    // Slider labels
    QWidget *labelsWidget = new QWidget;
    QHBoxLayout *labelsLayout = new QHBoxLayout(labelsWidget);
    labelsLayout->setContentsMargins(0, 0, 0, 0);
    
    m_pLeftHintLabel = new QLabel("0");
    m_pLeftHintLabel->setObjectName("SliderLabel");
    
    m_pRightHintLabel = new QLabel("100");
    m_pRightHintLabel->setObjectName("SliderLabel");
    
    labelsLayout->addWidget(m_pLeftHintLabel);
    labelsLayout->addStretch();
    labelsLayout->addWidget(m_pRightHintLabel);
    
    // Slider
    m_pAdjustSlider = new SliderClick(Qt::Horizontal);
    m_pAdjustSlider->setObjectName("BrightnessSlider");
    m_pAdjustSlider->setFixedHeight(30);
    
    sliderLayout->addWidget(labelsWidget);
    sliderLayout->addWidget(m_pAdjustSlider);
    
    // Preview section
   // m_pPreviewLabel = new QLabel;
   // m_pPreviewLabel->setObjectName("PreviewLabel");
   // m_pPreviewLabel->setAlignment(Qt::AlignCenter);
   // m_pPreviewLabel->setWordWrap(true);
    
    // Add all sections to card
    cardLayout->addWidget(m_pDisplayWidget);
    cardLayout->addWidget(m_pSliderWidget);
   // cardLayout->addWidget(m_pPreviewLabel);
    
    contentLayout->addWidget(m_pBrightnessCard);
}

void LuminanceFrmPrivate::InitData()
{
    q_func()->setAttribute(Qt::WA_QuitOnClose, false);
    
    m_pAdjustSlider->setMinimum(0);
    m_pAdjustSlider->setMaximum(100);
    m_pAdjustSlider->setValue(75);
    
    UpdateBrightnessDisplay(75);
}

void LuminanceFrmPrivate::InitConnect()
{
    QObject::connect(m_pAdjustSlider, &QSlider::valueChanged, q_func(), &LuminanceFrm::slotValueChanged);
    QObject::connect(m_pAdjustSlider, &QSlider::valueChanged, [this](int value) {
        UpdateBrightnessDisplay(value);
    });
}

void LuminanceFrmPrivate::UpdateBrightnessDisplay(int value)
{
    // Update value label
    m_pBrightnessValue->setText(QString::number(value));
    
    // Update left hint (current value)
    m_pLeftHintLabel->setText(QString::number(value));
    
    // Update preview text
  //  QString description = getBrightnessDescription(value);
   // m_pPreviewLabel->setText(QString("Current brightness: %1% - %2").arg(value).arg(description));
    
    // Update icon opacity based on brightness (simulate brightness effect)
    qreal opacity = 0.3 + (value / 100.0) * 0.7;
    m_pBrightnessIcon->setStyleSheet(QString("opacity: %1;").arg(opacity));
}

void LuminanceFrmPrivate::ApplyModernStyles()
{
    q_func()->setStyleSheet(
        // Main dialog
        "QDialog#LuminanceFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
"QWidget#DialogHeader {"
"    background: #2196F3;"  // Solid color instead of gradient
"    border-top-left-radius: 15px;"
"    border-top-right-radius: 15px;"
"}"
        "QLabel#DialogTitle {"
        "    color: white;"
        "    font-size: 24px;"
        "    font-weight: 600;"
        "}"
        
        // Content area
        "QWidget#ContentWidget {"
        "    background: white;"
        "}"
        
        // Brightness card
        "QFrame#BrightnessCard {"
        "    background: #f8f9fa;"
        "    border-radius: 12px;"
        "    border-left: 4px solid #2196F3;"
        "}"
        
        // Brightness display
        "QLabel#BrightnessIcon {"
        "    font-size: 48px;"
        "    margin-bottom: 15px;"
        "}"
        "QLabel#BrightnessValue {"
        "    font-size: 36px;"
        "    font-weight: 700;"
        "    color: #2c3e50;"
        "    margin-bottom: 5px;"
        "}"
        "QLabel#BrightnessLabel {"
        "    font-size: 14px;"
        "    color: #6c757d;"
        "    font-weight: 600;"
        "    letter-spacing: 1px;"
        "}"
        
        // Slider labels
        "QLabel#SliderLabel {"
        "    font-size: 14px;"
        "    color: #6c757d;"
        "    font-weight: 500;"
        "}"
        
        // Slider styling
        "QSlider#BrightnessSlider {"
        "    background: transparent;"
        "}"
        "QSlider#BrightnessSlider::groove:horizontal {"
        "    border: none;"
        "    height: 8px;"
        "    background: #e9ecef;"
        "    border-radius: 4px;"
        "}"
        "QSlider#BrightnessSlider::sub-page:horizontal {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "                stop:0 #2196F3, stop:1 #4CAF50);"
        "    border-radius: 4px;"
        "}"
        "QSlider#BrightnessSlider::handle:horizontal {"
        "    background: white;"
        "    border: 3px solid #2196F3;"
        "    width: 24px;"
        "    height: 24px;"
        "    border-radius: 12px;"
        "    margin-top: -8px;"
        "    margin-bottom: -8px;"
        "}"
        "QSlider#BrightnessSlider::handle:horizontal:hover {"
        "    border-color: #1976D2;"
        "    transform: scale(1.1);"
        "}"
        
    );
}

// Public interface methods
void LuminanceFrm::setAdjustValue(const int &value)
{
    Q_D(LuminanceFrm);
    d->m_pAdjustSlider->setValue(value);
    d->UpdateBrightnessDisplay(value);
}

int LuminanceFrm::getAdjustValue() const
{
    return d_func()->m_pAdjustSlider->value();
}

void LuminanceFrm::slotValueChanged(int value)
{
    Q_D(LuminanceFrm);
    
#ifdef Q_OS_LINUX
    YNH_LJX::Display::Display_SetScreenBrightness(value);
#endif
}

#ifdef SCREENCAPTURE
void LuminanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif