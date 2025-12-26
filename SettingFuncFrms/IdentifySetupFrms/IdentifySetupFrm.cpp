// Modified IdentifySetupFrm.cpp - Card-based theme with AGGRESSIVE dialog transparency fix

#include "IdentifySetupFrm.h"
#include "IdentifyDistanceFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"
#include "SettingFuncFrms/SetupItemDelegate/CInputBaseDialog.h"
#include "Config/ReadConfig.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QObject>
#include <QPixmap>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

// Helper function to AGGRESSIVELY force white background on dialogs
static void ForceDialogWhiteBackground(QDialog *dialog)
{
    if (!dialog) return;
    
    // Set all possible transparency-preventing attributes
    dialog->setAttribute(Qt::WA_OpaquePaintEvent, true);
    dialog->setAutoFillBackground(true);
    dialog->setAttribute(Qt::WA_TranslucentBackground, false);
    dialog->setAttribute(Qt::WA_NoSystemBackground, false);
    dialog->setAttribute(Qt::WA_StyledBackground, true);
    
    // Set palette with ALL color roles
    QPalette p = dialog->palette();
    p.setColor(QPalette::Window, Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Background, Qt::white);
    p.setColor(QPalette::AlternateBase, Qt::white);
    dialog->setPalette(p);
    
    // AGGRESSIVE stylesheet with !important
    dialog->setStyleSheet(
        "QDialog { "
        "    background-color: white !important; "
        "    background: white !important; "
        "} "
        "QWidget { "
        "    background-color: white !important; "
        "} "
        "QFrame { "
        "    background-color: white !important; "
        "} "
        "QLabel { "
        "    background-color: transparent; "
        "} "
        "QLineEdit { "
        "    background-color: white; "
        "    border: 1px solid #ccc; "
        "} "
        "QPushButton { "
        "    background-color: #e0e0e0; "
        "    border: 1px solid #999; "
        "    padding: 5px 15px; "
        "}"
    );
    
    // Force immediate update
    dialog->update();
    dialog->repaint();
    
    // Also set background on all child widgets
    QTimer::singleShot(0, [dialog]() {
        QList<QWidget*> children = dialog->findChildren<QWidget*>();
        foreach(QWidget* child, children) {
            if (!qobject_cast<QLabel*>(child) && 
                !qobject_cast<QLineEdit*>(child) && 
                !qobject_cast<QPushButton*>(child)) {
                child->setAutoFillBackground(true);
                QPalette cp = child->palette();
                cp.setColor(QPalette::Window, Qt::white);
                cp.setColor(QPalette::Background, Qt::white);
                child->setPalette(cp);
            }
        }
        dialog->update();
    });
}

// Forward declaration and helper function definition
static inline QString IdentifyDistanceToSTR(const int &index)
{
    switch(index)
    {
    case 0: return QString("50cm");
    case 1: return QString("100cm");
    default: return QString("150cm");
    }
}

// Custom Identity Setting Card Widget
class IdentitySettingCardWidget : public QFrame
{
    Q_OBJECT
public:
    enum CardType { ToggleCard, ValueCard };
    
    IdentitySettingCardWidget(const QString &title, CardType type, const QString &iconPath = "", QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_cardType(type), m_iconPath(iconPath)
    {
        setObjectName("IdentityCardWidget");
        setupUI(title, type, iconPath);
    }
    
    QString getTitle() const { return m_title; }
    
    void setValue(const QString &value)
    {
        if (m_cardType == ValueCard && m_valueLabel) {
            m_valueLabel->setText(value);
        }
    }
    
    void setToggleState(bool enabled)
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            m_toggleCheckBox->setChecked(enabled);
        }
    }
    
    bool getToggleState() const
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            return m_toggleCheckBox->isChecked();
        }
        return false;
    }

signals:
    void cardClicked(const QString &title);
    void toggleChanged(bool enabled);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (m_cardType == ValueCard) {
            emit cardClicked(m_title);
        }
        QFrame::mousePressEvent(event);
    }
    
    void enterEvent(QEvent *event) override
    {
        setProperty("hovered", true);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override
    {
        setProperty("hovered", false);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::leaveEvent(event);
    }
private:
    QString getIconText(const QString &title)
    {
        if (title.contains("Living") && title.contains("Detection"))
            return "";
        else if (title.contains("Living") && title.contains("Threshold"))
            return "";
        else if (title.contains("Comparison") && title.contains("Threshold"))
            return "";
        else if (title.contains("Face") && title.contains("Quality"))
            return "";
        else if (title.contains("Identification") && title.contains("Interval"))
            return "";
        else if (title.contains("Recognition") && title.contains("Distance"))
            return "";
        return "";
    }

private:
    void setupUI(const QString &title, CardType type, const QString &iconPath)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(12, 10, 12, 10);
        mainLayout->setSpacing(12);
        
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(24, 24);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("IdentityIconLabel");
        
        if (!iconPath.isEmpty()) {
            QPixmap iconPixmap(iconPath);
            if (!iconPixmap.isNull()) {
                iconPixmap = iconPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                iconLabel->setPixmap(iconPixmap);
            } else {
                iconLabel->setText(getIconText(title));
            }
        } else {
            iconLabel->setText(getIconText(title));
        }
        
        iconLayout->addWidget(iconLabel);
        
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(3);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("IdentityTitleLabel");
        textLayout->addWidget(titleLabel);
        
        if (type == ValueCard) {
            m_valueLabel = new QLabel("--");
            m_valueLabel->setObjectName("IdentityValueLabel");
            textLayout->addWidget(m_valueLabel);
        }
        
        QHBoxLayout *rightLayout = new QHBoxLayout;
        rightLayout->setSpacing(10);
        
        if (type == ToggleCard) {
            m_toggleCheckBox = new QCheckBox;
            m_toggleCheckBox->setObjectName("IdentityToggleBox");
            QObject::connect(m_toggleCheckBox, &QCheckBox::toggled, this, &IdentitySettingCardWidget::toggleChanged);
            rightLayout->addWidget(m_toggleCheckBox);
        } else {
            QLabel *arrowLabel = new QLabel("›");
            arrowLabel->setObjectName("IdentityArrowLabel");
            rightLayout->addWidget(arrowLabel);
        }
        
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addLayout(rightLayout);
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Living") || title.contains("Detection"))
            return "BiometricIconFrame";
        else if (title.contains("Threshold") || title.contains("Quality"))
            return "ThresholdIconFrame";
        else if (title.contains("Interval") || title.contains("Distance"))
            return "SettingsIconFrame";
        return "DefaultIconFrame";
    }

private:
    QString m_title;
    CardType m_cardType;
    QString m_iconPath;
    QLabel *m_valueLabel = nullptr;
    QCheckBox *m_toggleCheckBox = nullptr;
};

class IdentifySetupFrmPrivate
{
    Q_DECLARE_PUBLIC(IdentifySetupFrm)
public:
    IdentifySetupFrmPrivate(IdentifySetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
public:
    bool CheckTopWidget();
private:
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<IdentitySettingCardWidget*> m_cardWidgets;
private:
    IdentifySetupFrm *const q_ptr;
};

IdentifySetupFrmPrivate::IdentifySetupFrmPrivate(IdentifySetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void IdentifySetupFrmPrivate::InitUI()
{
    Q_Q(IdentifySetupFrm);
    
    q->setObjectName("IdentifySetupMainWidget");
    
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(12, 12, 12, 12); 
    m_pMainLayout->setSpacing(0);
    
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("IdentifySetupScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("IdentifySetupContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(6);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    m_pMainLayout->addWidget(m_pScrollArea);
}

void IdentifySetupFrmPrivate::InitData()
{
    QStringList cardTitles = {
        QObject::tr("LivingBodyDetection"),
        QObject::tr("LivingThreshold"),
        QObject::tr("ComparisonThreshold"),
        QObject::tr("FaceQualityThreshold"),
        QObject::tr("IdentificationInterval"),
        QObject::tr("RecognitionDistance")
    };
    
    QStringList iconPaths = {
        ":/icons/living_detection.png",
        ":/icons/living_threshold.png",
        ":/icons/comparison_threshold.png",
        ":/icons/face_quality.png",
        ":/icons/identification_interval.png",
        ":/icons/recognition_distance.png"
    };
    
    for(int i = 0; i < cardTitles.count(); i++)
    {
        IdentitySettingCardWidget::CardType type = (i == 0) ? 
            IdentitySettingCardWidget::ToggleCard : 
            IdentitySettingCardWidget::ValueCard;
            
        QString iconPath = (i < iconPaths.count()) ? iconPaths.at(i) : "";
        IdentitySettingCardWidget *cardWidget = new IdentitySettingCardWidget(cardTitles.at(i), type, iconPath);
        
        switch(i) {
            case 0:
                cardWidget->setToggleState(ReadConfig::GetInstance()->getIdentity_Manager_CheckLiving());
                break;
            case 1:
                cardWidget->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Living_value()));
                break;
            case 2:
                cardWidget->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Thanthreshold()));
                break;
            case 3:
                cardWidget->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold()));
                break;
            case 4:
                cardWidget->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_IdentifyInterval()) + "s");
                break;
            case 5:
                cardWidget->setValue(IdentifyDistanceToSTR(ReadConfig::GetInstance()->getIdentity_Manager_Identifycm()));
                break;
        }
        
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    m_pContentLayout->addStretch();
}

void IdentifySetupFrmPrivate::InitConnect()
{
    for(int i = 0; i < m_cardWidgets.count(); i++)
    {
        IdentitySettingCardWidget *card = m_cardWidgets[i];
        
        if (i == 0) {
            QObject::connect(card, &IdentitySettingCardWidget::toggleChanged, 
                   [](bool enabled) {
                ReadConfig::GetInstance()->setIdentity_Manager_CheckLiving(enabled ? 1 : 0);
            });
        } else {
            QObject::connect(card, &IdentitySettingCardWidget::cardClicked, 
                   [this, i](const QString &title) {
                Q_UNUSED(title);
                Q_Q(IdentifySetupFrm);
                q->handleCardClicked(i);
            });
        }
    }
}

bool IdentifySetupFrmPrivate::CheckTopWidget()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog") return true;
    }
    return false;
}

IdentifySetupFrm::IdentifySetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new IdentifySetupFrmPrivate(this))
{
}

IdentifySetupFrm::~IdentifySetupFrm()
{
}

void IdentifySetupFrm::setEnter()
{
    Q_D(IdentifySetupFrm);
    
    if (d->m_cardWidgets.count() >= 6) {
        d->m_cardWidgets[0]->setToggleState(ReadConfig::GetInstance()->getIdentity_Manager_CheckLiving());
        d->m_cardWidgets[1]->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Living_value()));
        d->m_cardWidgets[2]->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Thanthreshold()));
        d->m_cardWidgets[3]->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold()));
        d->m_cardWidgets[4]->setValue(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_IdentifyInterval()) + "s");
        d->m_cardWidgets[5]->setValue(IdentifyDistanceToSTR(ReadConfig::GetInstance()->getIdentity_Manager_Identifycm()));
    }
}

void IdentifySetupFrm::setLeaveEvent()
{
    Q_D(IdentifySetupFrm);
    
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog")
        {
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }

    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void IdentifySetupFrm::handleCardClicked(int cardIndex)
{
    Q_D(IdentifySetupFrm);
    
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
#endif    

    if(d->CheckTopWidget()) return;
    
    switch(cardIndex) {
        case 1: // Living Threshold
        {
            CInputBaseDialog dlg(this);
            ForceDialogWhiteBackground(&dlg);  // USE HELPER FUNCTION
            
            dlg.setTitleText(QObject::tr("LivingThreshold"));
            dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
            dlg.setFloatValidator(0.0, 1.0);
            dlg.show();
            
            // Force again after show
            ForceDialogWhiteBackground(&dlg);
            
            if(dlg.exec() == 0)
            {
                QString data = dlg.getData();
                ReadConfig::GetInstance()->setIdentity_Manager_Living_value(data.toFloat());
                d->m_cardWidgets[1]->setValue(data);
            }
            break;
        }
        case 2: // Comparison Threshold
        {
            CInputBaseDialog dlg(this);
            ForceDialogWhiteBackground(&dlg);
            
            dlg.setTitleText(QObject::tr("ComparisonThreshold"));
            dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
            dlg.setFloatValidator(0.0, 1.0);
            dlg.show();
            
            ForceDialogWhiteBackground(&dlg);
            
            if(dlg.exec() == 0)
            {
                QString data = dlg.getData();
                ReadConfig::GetInstance()->setIdentity_Manager_Thanthreshold(data.toFloat());
                d->m_cardWidgets[2]->setValue(data);
            }
            break;
        }
        case 3: // Face Quality Threshold
        {
            CInputBaseDialog dlg(this);
            ForceDialogWhiteBackground(&dlg);
            
            dlg.setTitleText(QObject::tr("FaceQualityThreshold"));
            dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
            dlg.setFloatValidator(0.0, 1.0);
            dlg.show();
            
            ForceDialogWhiteBackground(&dlg);
            
            if(dlg.exec() == 0)
            {
                QString data = dlg.getData();
                ReadConfig::GetInstance()->setIdentity_Manager_FqThreshold(data.toFloat());
                d->m_cardWidgets[3]->setValue(data);
            }
            break;
        }
        case 4: // Identification Interval
        {
            CInputBaseDialog dlg(this);
            ForceDialogWhiteBackground(&dlg);
            
            dlg.setTitleText(QObject::tr("IdentificationInterval"));
            dlg.setPlaceholderText(QObject::tr("0~7200s"));
            dlg.setIntValidator(0, 7200);
            dlg.show();
            
            ForceDialogWhiteBackground(&dlg);
            
            if(dlg.exec() == 0)
            {
                QString data = dlg.getData();
                ReadConfig::GetInstance()->setIdentity_Manager_IdentifyInterval(data.toInt());
                d->m_cardWidgets[4]->setValue(data + "s");
            }
            break;
        }
        case 5: // Recognition Distance
        {
            IdentifyDistanceFrm dlg(this);
            ForceDialogWhiteBackground(&dlg);
            
            dlg.show();
            ForceDialogWhiteBackground(&dlg);
            
            if(dlg.exec() == 0)
            {
                ReadConfig::GetInstance()->setIdentity_Manager_Identifycm(dlg.getDistanceMode());
                d->m_cardWidgets[5]->setValue(IdentifyDistanceToSTR(dlg.getDistanceMode()));
            }
            break;
        }
    }
}

void IdentifySetupFrm::slotIemClicked(QListWidgetItem *item)
{
}

#ifdef SCREENCAPTURE
void IdentifySetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif

#include "IdentifySetupFrm.moc"