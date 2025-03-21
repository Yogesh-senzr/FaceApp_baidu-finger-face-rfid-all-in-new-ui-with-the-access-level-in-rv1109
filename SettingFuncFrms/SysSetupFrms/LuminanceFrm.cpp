#include "LuminanceFrm.h"
#include "sliderclick.h"
#ifdef Q_OS_LINUX
#include "PCIcore/Display.h"
#endif
#include <QLabel>
#include <QHBoxLayout>

class LuminanceFrmPrivate
{
    Q_DECLARE_PUBLIC(LuminanceFrm)
public:
    LuminanceFrmPrivate(LuminanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
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
}

LuminanceFrm::LuminanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LuminanceFrmPrivate(this))
{

}

LuminanceFrm::~LuminanceFrm()
{

}

void LuminanceFrmPrivate::InitUI()
{
    m_pLeftHintLabel = new QLabel("0");
    m_pAdjustSlider = new SliderClick(Qt::Horizontal);
    m_pRightHintLabel = new QLabel("100");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("BrightnessSetting")));//亮度设置
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(10);
    layout1->addWidget(m_pLeftHintLabel);
    layout1->addWidget(m_pAdjustSlider);
    layout1->addSpacing(8);
    layout1->addWidget(m_pRightHintLabel);
    layout1->addSpacing(10);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    m_pLeftHintLabel->setObjectName("LuminanceFrmLabel");
    m_pRightHintLabel->setObjectName("LuminanceFrmLabel");

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(0, 5, 0, 0);
    vLayout->addLayout(layout);
    vLayout->addLayout(layout1);
    vLayout->addStretch();
}

void LuminanceFrmPrivate::InitData()
{
    q_func()->setAttribute(Qt::WA_QuitOnClose, false);
    q_func()->setObjectName("LuminanceFrm");
    m_pAdjustSlider->setMinimum(0);
    m_pAdjustSlider->setMaximum(100);
}

void LuminanceFrmPrivate::InitConnect()
{
    QObject::connect(m_pAdjustSlider, &QSlider::valueChanged, q_func(), &LuminanceFrm::slotValueChanged);
}

void LuminanceFrm::setAdjustValue(const int &value)
{
    Q_D(LuminanceFrm);
    d->m_pAdjustSlider->setValue(value);
}

int LuminanceFrm::getAdjustValue()const
{
    return d_func()->m_pAdjustSlider->value();
}

void LuminanceFrm::slotValueChanged(int value)
{
    Q_D(LuminanceFrm);
    d->m_pLeftHintLabel->setText(QString::number(value));
#ifdef Q_OS_LINUX
    YNH_LJX::Display::Display_SetScreenBrightness(value);
#endif
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void LuminanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 