#include "IdentifySetupFrm.h"

#include "IdentifyDistanceFrm.h"
#include "TemperatureModeFrm.h"

#include "../SetupItemDelegate/CItemWidget.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"
#include "SettingFuncFrms/SetupItemDelegate/CInputBaseDialog.h"
#include "Config/ReadConfig.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>

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
    QListWidget *m_pListWidget;
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

IdentifySetupFrm::IdentifySetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new IdentifySetupFrmPrivate(this))
{

}

IdentifySetupFrm::~IdentifySetupFrm()
{

}

void IdentifySetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void IdentifySetupFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 480:spacing = 14;break;		
    case 600:spacing = 14;break;
    case 720:spacing = 26;break;
    case 800:spacing = 0;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("LivingBodyDetection"), ":/Images/SmallRound.png");//"活体检测"
		if (width ==480)
			pItemWidget->setAddSpacing(120);
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("LivingThreshold"), ":/Images/SmallRound.png", "0.7");//活体阈值
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("ComparisonThreshold"), ":/Images/SmallRound.png", "0.8");//比对阈值
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("MaskThreshold"), ":/Images/SmallRound.png", "0.25");//口罩阈值
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("Person-witnessComparisonThreshold"), ":/Images/SmallRound.png", "0.7");//人证比对阈值
		if (width ==480)
			pItemWidget->setAddSpacing(120);
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("FaceQualityThreshold"), ":/Images/SmallRound.png", "0.5");//人脸质量阈值
		if (width ==480)
			pItemWidget->setAddSpacing(120);
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("IdentificationInterval"), ":/Images/SmallRound.png", "1s");//识别间隔
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("RecognitionDistance"), ":/Images/SmallRound.png", "150cm");//识别距离
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("TemperatureMeasuringEnvironment "), ":/Images/SmallRound.png", QObject::tr("outdoor"));//测温环境,户外
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("temperatureCompensation"), ":/Images/SmallRound.png", "0.0");//温度补偿
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(10);

        pItemWidget->setData(QObject::tr("AlarmTemperature"), ":/Images/SmallRound.png", "37.3");//报警温度
		if (width ==480)
			pItemWidget->setAddSpacing(120);		
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void IdentifySetupFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &IdentifySetupFrm::slotIemClicked);
}

bool IdentifySetupFrmPrivate::CheckTopWidget()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog")return true;
    }
    return false;
}

static inline QString IdentifyDistanceToSTR(const int &index)
{
    switch(index)
    {
    case 0: return QString("50cm");
    case 1: return QString("100cm");
    default :return QString("150cm");
    }
}
static inline QString TemperatureModeToSTR(const int &index)
{
    switch(index)
    {
    case 0: return QObject::tr("interior");//室内
    case 1: return QObject::tr("outdoor");//户外
    default :return QObject::tr("interior");//室内
    }
}

void IdentifySetupFrm::setEnter()
{
    Q_D(IdentifySetupFrm);
    //活体检测
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->setCheckBoxState(ReadConfig::GetInstance()->getIdentity_Manager_CheckLiving());
    //活体阈值
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(1)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Living_value()));
    //比对阈值
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Thanthreshold()));
    //口罩阈值
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_Mask_value()));
    //身份证比对阈值
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(4)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_idcardThanthreshold()));
    //人脸质量阈值
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(5)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold()));
    //识别间隔
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(6)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_IdentifyInterval()) + "s");
    //识别距离
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(7)))->setRNameText(IdentifyDistanceToSTR(ReadConfig::GetInstance()->getIdentity_Manager_Identifycm()));
    //测温环境
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(8)))->setRNameText(TemperatureModeToSTR(ReadConfig::GetInstance()->getIdentity_Manager_TemperatureMode()));
    //温度补偿
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(9)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_TempComp()));
    //报警温度
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(10)))->setRNameText(QString::number(ReadConfig::GetInstance()->getIdentity_Manager_AlarmTemp()));
}

void IdentifySetupFrm::setLeaveEvent()
{
    Q_D(IdentifySetupFrm);
    //活体检测
    ReadConfig::GetInstance()->setIdentity_Manager_CheckLiving(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->getCheckBoxState() ? 1 : 0);
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog")
        {//如果存在顶层用户未关闭的窗口， 使其不可见
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }

    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void IdentifySetupFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(IdentifySetupFrm);
#ifdef SCREENCAPTURE  //ScreenCapture       
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
#endif    
    if(d->CheckTopWidget())return;
    else if(item == d->m_pListWidget->item(1))
    {//活体阈值
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("LivingThreshold"));////活体阈值
        dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
        dlg.setFloatValidator(0.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_Living_value(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(2))
    {//比对阈值
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("ComparisonThreshold"));//"比对阈值"
        dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
        dlg.setFloatValidator(0.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_Thanthreshold(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(3))
    {//口罩阈值
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("MaskThreshold"));//"口罩阈值"
        dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
        dlg.setFloatValidator(0.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_Mask_value(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(4))
    {//身份证比对阈值
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("Person-witnessComparisonThreshold"));//"人证比对阈值"
        dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
        dlg.setFloatValidator(0.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_idcardThanthreshold(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(5))
    {//人脸质量阈值
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("FaceQualityThreshold"));//"人脸质量阈值"
        dlg.setPlaceholderText(QObject::tr("0.0~1.0"));
        dlg.setFloatValidator(0.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_FqThreshold(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(6))
    {//识别间隔
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("IdentificationInterval"));//"识别间隔"
        dlg.setPlaceholderText(QObject::tr("0~7200s"));
        dlg.setIntValidator(0, 7200);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_IdentifyInterval(data.toInt());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data + "s");
        }
    }else if(item == d->m_pListWidget->item(7))
    {//识别距离
        IdentifyDistanceFrm dlg(this);
        if(dlg.exec() == 0)
        {
            ReadConfig::GetInstance()->setIdentity_Manager_Identifycm(dlg.getDistanceMode());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(IdentifyDistanceToSTR(dlg.getDistanceMode()));
        }
    }else if(item == d->m_pListWidget->item(8))
    {//测温环境
        TemperatureModeFrm dlg(this);
        if(dlg.exec() == 0)
        {
            ReadConfig::GetInstance()->setIdentity_Manager_TemperatureMode(dlg.getTemperatureMode());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(TemperatureModeToSTR(dlg.getTemperatureMode()));
        }
    }else if(item == d->m_pListWidget->item(9))
    {//温度补偿
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("temperatureCompensation"));//温度补偿
        dlg.setPlaceholderText("-1.0~1.0");
        dlg.setFloatValidator(-1.0, 1.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_TempComp(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }else if(item == d->m_pListWidget->item(10))
    {//报警温度
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("AlarmTemperature"));//报警温度
        dlg.setPlaceholderText(QObject::tr("Suchas:37.3"));//QObject::tr("例如:37.3")
        dlg.setFloatValidator(0.0, 100.0);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setIdentity_Manager_AlarmTemp(data.toFloat());
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data);
        }
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void IdentifySetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif