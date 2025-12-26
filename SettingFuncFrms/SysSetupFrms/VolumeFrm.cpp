#include "VolumeFrm.h"
#include "sliderclick.h"

#include "PCIcore/Audio.h"
#include "Config/ReadConfig.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QPushButton>

class VolumeFrmPrivate
{
    Q_DECLARE_PUBLIC(VolumeFrm)
public:
    VolumeFrmPrivate(VolumeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Slider container
    QWidget *m_pSliderContainer;
    QLabel *m_pLeftHintLabel;
    SliderClick *m_pAdjustSlider;
    QLabel *m_pRightHintLabel;
    QPushButton *m_pAdd;
    QPushButton *m_pReduce;
    
    // Action buttons
    QWidget *m_pActionsWidget;
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    VolumeFrm *const q_ptr;
};

VolumeFrmPrivate::VolumeFrmPrivate(VolumeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

VolumeFrm::VolumeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new VolumeFrmPrivate(this))
{

}

VolumeFrm::~VolumeFrm()
{

}

void VolumeFrmPrivate::InitUI()
{
    q_func()->setObjectName("VolumeFrm");
    // Increased size from 400x120 to 500x160 for more spacious layout
    q_func()->setFixedSize(600, 180);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section - increased height proportionally
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(80); // Increased from 70
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(25, 0, 25, 0); // Increased margins
    
    m_pTitleLabel = new QLabel(QObject::tr("Volume Setting"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30); // Increased margins
    contentLayout->setSpacing(30); // Increased spacing
    
    // Slider container
    m_pSliderContainer = new QWidget;
    QHBoxLayout *sliderLayout = new QHBoxLayout(m_pSliderContainer);
    sliderLayout->setSpacing(20); // Increased spacing
    
    m_pLeftHintLabel = new QLabel("0");
    m_pLeftHintLabel->setObjectName("SliderValue");
    
    m_pReduce = new QPushButton("−");
    m_pReduce->setObjectName("ControlButton");
    
    m_pAdjustSlider = new SliderClick(Qt::Horizontal);
    m_pAdjustSlider->setObjectName("VolumeSlider");
    
    m_pAdd = new QPushButton("+");
    m_pAdd->setObjectName("ControlButton");
    
    m_pRightHintLabel = new QLabel("100");
    m_pRightHintLabel->setObjectName("SliderValue");
    
    sliderLayout->addWidget(m_pLeftHintLabel);
    sliderLayout->addWidget(m_pReduce);
    sliderLayout->addWidget(m_pAdjustSlider, 1);
    sliderLayout->addWidget(m_pAdd);
    sliderLayout->addWidget(m_pRightHintLabel);
    
    // Action buttons section
    m_pActionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(m_pActionsWidget);
    actionsLayout->setContentsMargins(0, 25, 0, 0); // Increased top margin
    actionsLayout->setSpacing(20); // Increased spacing
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
    
    contentLayout->addWidget(m_pSliderContainer);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(m_pActionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void VolumeFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#VolumeFrm {"
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
        
        // Slider styles
        "QLabel#SliderValue {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "    min-width: 35px;"
        "}"
        "SliderClick#VolumeSlider {"
        "    height: 6px;"
        "}"
        "SliderClick#VolumeSlider::groove:horizontal {"
        "    background: #e1e5e9;"
        "    height: 6px;"
        "    border-radius: 3px;"
        "}"
        "SliderClick#VolumeSlider::handle:horizontal {"
        "    background: #2196F3;"
        "    width: 20px;"
        "    height: 20px;"
        "    border-radius: 10px;"
        "    margin: -7px 0;"
        "}"
        "SliderClick#VolumeSlider::sub-page:horizontal {"
        "    background: #2196F3;"
        "    border-radius: 3px;"
        "}"
        
        // Control button styles
        "QPushButton#ControlButton {"
        "    width: 40px;"
        "    height: 40px;"
        "    border-radius: 20px;"
        "    border: none;"
        "    background: #f8f9fa;"
        "    color: #2196F3;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#ControlButton:hover {"
        "    background: #2196F3;"
        "    color: white;"
        "}"
        
        // Action button styles
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

void VolumeFrmPrivate::InitData()
{
    q_func()->setAttribute(Qt::WA_QuitOnClose, false);
    m_pAdjustSlider->setMinimum(0);
    m_pAdjustSlider->setMaximum(100);
}

void VolumeFrmPrivate::InitConnect()
{
    QObject::connect(m_pAdjustSlider, &QSlider::valueChanged, q_func(), &VolumeFrm::slotValueChanged);
    QObject::connect(m_pAdjustSlider, &QSlider::sliderReleased, q_func(), &VolumeFrm::slotSliderReleased);

    QObject::connect(m_pReduce, &QPushButton::clicked, [this] {       
        int mValue = m_pAdjustSlider->value();
        if (mValue > 0) {
            mValue--;
            m_pAdjustSlider->setValue(mValue);
        }
    });    
    
    QObject::connect(m_pAdd, &QPushButton::clicked, [this] {  
        int mValue = m_pAdjustSlider->value();     
        if (mValue < 100) {
            mValue++;
            m_pAdjustSlider->setValue(mValue);
        }
    });
    
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [this] {
        q_func()->accept();
    });
    
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [this] {
        q_func()->reject();
    });
}

void VolumeFrm::setAdjustValue(const int &value)
{
    Q_D(VolumeFrm);
    d->m_pAdjustSlider->setValue(value);
}

int VolumeFrm::getAdjustValue() const
{
    return d_func()->m_pAdjustSlider->value();
}

void VolumeFrm::slotValueChanged(int value)
{
    Q_D(VolumeFrm);
    d->m_pLeftHintLabel->setText(QString::number(value));
    ReadConfig::GetInstance()->setVolume_Value(value);
}

void VolumeFrm::slotSliderReleased()
{
#ifdef Q_OS_LINUX
    YNH_LJX::Audio::Audio_PlayMedia_volume();
#endif
}

bool VolumeFrm::event(QEvent *event)
{
    if(event->type() == QEvent::User + 2) {
#ifdef Q_OS_LINUX
        YNH_LJX::Audio::Audio_PlayMedia_volume();
#endif
    }
    return QWidget::event(event);
}

#ifdef SCREENCAPTURE
void VolumeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif