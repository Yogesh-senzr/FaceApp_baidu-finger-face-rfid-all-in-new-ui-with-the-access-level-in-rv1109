#include "PassageModeFrm.h"

#include "Config/ReadConfig.h"
#include "OperationTipsFrm.h"

#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

class PassageModeFrmPrivate
{
    Q_DECLARE_PUBLIC(PassageModeFrm)
public:
    PassageModeFrmPrivate(PassageModeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void DealMust(const QString &);//处理必须项
    void DealOptional(const QString &);//处理可选项
private:
    //刷卡
    QCheckBox *m_pMust_SwipingicCardBox;
    //刷脸
    QCheckBox *m_pMust_SwipingFaceBox;
    //测温
    QCheckBox *m_pMust_ThermometryBox;
    //口罩
    QCheckBox *m_pMust_MaskBox;
    //二维码
    QCheckBox *m_pMust_QRcodeBox;
    //身份证
    QCheckBox *m_pMust_IdCardBox;
    //人证比对
    QCheckBox *m_pMust_IdCardComparisonBox;
private:
    //刷卡
    QCheckBox *m_pOptional_SwipingicCardBox;
    //刷脸
    QCheckBox *m_pOptional_SwipingFaceBox;
    //测温
    QCheckBox *m_pOptional_ThermometryBox;
    //口罩
    QCheckBox *m_pOptional_MaskBox;
    //二维码
    QCheckBox *m_pOptional_QRcodeBox;
    //身份证
    QCheckBox *m_pOptional_IdCardBox;
private:
    QPushButton *m_pSaveButton;
    QLabel *m_pLabel_Memo;
private:
    PassageModeFrm *const q_ptr;
};

PassageModeFrmPrivate::PassageModeFrmPrivate(PassageModeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

PassageModeFrm::PassageModeFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new PassageModeFrmPrivate(this))
{

}

PassageModeFrm::~PassageModeFrm()
{

}

void PassageModeFrmPrivate::InitUI()
{
    m_pSaveButton= new QPushButton(QObject::tr("Save"));//保存

    m_pLabel_Memo = new QLabel();

    m_pMust_SwipingicCardBox = new QCheckBox(QObject::tr("SwipingCard"));//刷卡
    m_pMust_SwipingFaceBox = new QCheckBox(QObject::tr("FaceSwiping"));//刷脸
    m_pMust_ThermometryBox = new QCheckBox(QObject::tr("MeasureTheTemperature"));//测温
    m_pMust_MaskBox = new QCheckBox(QObject::tr("BreathingMask"));//口罩
    m_pMust_QRcodeBox = new QCheckBox(QObject::tr("QRCode"));//二维码
    m_pMust_IdCardBox = new QCheckBox(QObject::tr("IdCard"));//身份证
    m_pMust_IdCardComparisonBox = new QCheckBox(QObject::tr("IdCardComparison"));//人证比对

    m_pOptional_SwipingicCardBox = new QCheckBox(QObject::tr("SwipingCard"));//刷卡
    m_pOptional_SwipingFaceBox = new QCheckBox(QObject::tr("FaceSwiping"));//刷脸
    m_pOptional_ThermometryBox = new QCheckBox(QObject::tr("MeasureTheTemperature"));//测温
    m_pOptional_MaskBox = new QCheckBox(QObject::tr("BreathingMask"));//口罩
    m_pOptional_QRcodeBox = new QCheckBox(QObject::tr("QRCode"));//二维码
    m_pOptional_IdCardBox = new QCheckBox(QObject::tr("IdCard"));//身份证

    QHBoxLayout *btnlayout = new QHBoxLayout;
    btnlayout->addStretch();
    btnlayout->addWidget(m_pSaveButton);
    btnlayout->addStretch();
    QGridLayout *hlayout = new QGridLayout; //必需项
    QGridLayout *hlayout1 = new QGridLayout; //可选项
    QGridLayout *hlayout2 = new QGridLayout; //备注

    int width = QApplication::desktop()->screenGeometry().width();
    switch(width)
    {
    case 600:
    {
        hlayout->addWidget(m_pMust_SwipingicCardBox, 0, 0);
        hlayout->addWidget(m_pMust_SwipingFaceBox, 0, 1);
        hlayout->addWidget(m_pMust_ThermometryBox, 0, 2);

        hlayout->addWidget(m_pMust_MaskBox, 1, 0);
        hlayout->addWidget(m_pMust_QRcodeBox, 1, 1);
        hlayout->addWidget(m_pMust_IdCardBox, 1, 2);

        hlayout->addWidget(m_pMust_IdCardComparisonBox, 2, 0);


        hlayout1->addWidget(m_pOptional_SwipingicCardBox, 0, 0);
        hlayout1->addWidget(m_pOptional_SwipingFaceBox, 0, 1);
        hlayout1->addWidget(m_pOptional_ThermometryBox, 0, 2);

        hlayout1->addWidget(m_pOptional_MaskBox, 1, 0);
        hlayout1->addWidget(m_pOptional_QRcodeBox, 1, 1);
        hlayout1->addWidget(m_pOptional_IdCardBox, 1, 2);
    }break;
    default:
    {
        hlayout->addWidget(m_pMust_SwipingicCardBox, 0, 0);
        hlayout->addWidget(m_pMust_SwipingFaceBox, 0, 1);
        hlayout->addWidget(m_pMust_ThermometryBox, 0, 2);
        hlayout->addWidget(m_pMust_MaskBox, 0, 3);

        hlayout->addWidget(m_pMust_QRcodeBox, 1, 0);
        hlayout->addWidget(m_pMust_IdCardBox, 1, 1);
        hlayout->addWidget(m_pMust_IdCardComparisonBox, 1, 2);


        hlayout1->addWidget(m_pOptional_SwipingicCardBox, 0, 0);
        hlayout1->addWidget(m_pOptional_SwipingFaceBox, 0, 1);
        hlayout1->addWidget(m_pOptional_ThermometryBox, 0, 2);
        hlayout1->addWidget(m_pOptional_MaskBox, 0, 3);
        hlayout1->addWidget(m_pOptional_QRcodeBox, 1, 0);
        hlayout1->addWidget(m_pOptional_IdCardBox, 1, 1);

        hlayout2->addWidget(m_pLabel_Memo, 0, 0);
    }break;
    }

    QGroupBox *group = new QGroupBox(QObject::tr("Required"));//必须满足项
    group->setLayout(hlayout);

    QGroupBox *group1 = new QGroupBox(QObject::tr("Optional"));//可选满足项
    group1->setLayout(hlayout1);


    QGroupBox *group2 = new QGroupBox(QObject::tr("memo"));//备注    
    group2->setLayout(hlayout2);
#if 0    
    m_pLabel_Memo->setText(QObject::tr("1.必填项:该必填项一定要处理,要满足。 \n\n " \
                                       "2. 选填项:在满足必填项后,满足其中之一的选填项 \n" \
                                       "      则可通过处理(可开门)。\n\n " \
                                       "3. 二维码(国康码),身份证 必须有外网才使用\n" )
                            );
#endif                             
    m_pLabel_Memo->setText(QObject::tr("MemoTipInfo"));

    m_pLabel_Memo->setStyleSheet("color:blue;");


    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->addWidget(group);
    vlayout->addWidget(group1);
    vlayout->addWidget(group2);
    vlayout->addSpacing(10);
    //vlayout->addLayout(btnlayout);
    vlayout->addStretch();
}

void PassageModeFrmPrivate::InitData()
{
    m_pSaveButton->setFixedSize(258, 72);
}

void PassageModeFrmPrivate::InitConnect()
{
    QObject::connect(m_pSaveButton, &QPushButton::clicked, q_func(), &PassageModeFrm::slotSaveButton);

    QObject::connect(m_pMust_SwipingicCardBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_SwipingFaceBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_ThermometryBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_MaskBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_QRcodeBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_IdCardBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
    QObject::connect(m_pMust_IdCardComparisonBox, &QCheckBox::stateChanged, q_func(), &PassageModeFrm::slotMustCheckBox);
}

void PassageModeFrmPrivate::DealMust(const QString &text)
{
    m_pMust_SwipingicCardBox->setChecked(text.contains("1"));
    m_pMust_SwipingFaceBox->setChecked(text.contains("2"));
    m_pMust_ThermometryBox->setChecked(text.contains("3"));
    m_pMust_MaskBox->setChecked(text.contains("4"));
    m_pMust_QRcodeBox->setChecked(text.contains("5"));
    m_pMust_IdCardBox->setChecked(text.contains("6"));
    m_pMust_IdCardComparisonBox->setChecked(text.contains("7"));
}

void PassageModeFrmPrivate::DealOptional(const QString &text)
{
    m_pOptional_SwipingicCardBox->setChecked(text.contains("1"));
    m_pOptional_SwipingFaceBox->setChecked(text.contains("2"));
    m_pOptional_ThermometryBox->setChecked(text.contains("3"));
    m_pOptional_MaskBox->setChecked(text.contains("4"));
    m_pOptional_QRcodeBox->setChecked(text.contains("5"));
    m_pOptional_IdCardBox->setChecked(text.contains("6")); 
}

bool PassageModeFrm::getOpenIsEmpty()
{
    Q_D(PassageModeFrm);
//    return ((d->m_pMust_SwipingicCardBox->checkState() == Qt::Unchecked) && (d->m_pMust_SwipingFaceBox->checkState() == Qt::Unchecked)
//            && (d->m_pMust_ThermometryBox->checkState() == Qt::Unchecked) && (d->m_pMust_MaskBox->checkState() == Qt::Unchecked)
//            && (d->m_pMust_QRcodeBox->checkState() == Qt::Unchecked) && (d->m_pMust_IdCardBox->checkState() == Qt::Unchecked)
//            && (d->m_pMust_IdCardComparisonBox->checkState() == Qt::Unchecked)) ? true : false;
    return false;
}

void PassageModeFrm::setEnter()
{
    Q_D(PassageModeFrm);
    d->DealMust(ReadConfig::GetInstance()->getDoor_MustOpenMode());
    d->DealOptional(ReadConfig::GetInstance()->getDoor_OptionalOpenMode());
}

void PassageModeFrm::setLeaveEvent()
{
    this->slotSaveButton();
}

void PassageModeFrm::slotMustCheckBox(const int &state)
{
    Q_D(PassageModeFrm);
    if(d->m_pMust_SwipingicCardBox == sender())
    {
        if(state)d->m_pOptional_SwipingicCardBox->setChecked(false);
        d->m_pOptional_SwipingicCardBox->setEnabled(!state);
    }
    else if(d->m_pMust_SwipingFaceBox == sender())
    {
        if(state)d->m_pOptional_SwipingFaceBox->setChecked(false);
        d->m_pOptional_SwipingFaceBox->setEnabled(!state);

        if (d->m_pMust_SwipingFaceBox->checkState() == Qt::Unchecked)
          d->m_pMust_IdCardComparisonBox->setChecked(false);
    }
    else if(d->m_pMust_ThermometryBox == sender())
    {
        if(state)d->m_pOptional_ThermometryBox->setChecked(false);
        d->m_pOptional_ThermometryBox->setEnabled(!state);
    }
    else if(d->m_pMust_MaskBox == sender())
    {
        if(state)d->m_pOptional_MaskBox->setChecked(false);
        d->m_pOptional_MaskBox->setEnabled(!state);
    }
    else if(d->m_pMust_QRcodeBox == sender())
    {
        if(state)d->m_pOptional_QRcodeBox->setChecked(false);
        d->m_pOptional_QRcodeBox->setEnabled(!state);
    }
    else if(d->m_pMust_IdCardBox == sender())
    {
        if(state)d->m_pOptional_IdCardBox->setChecked(false);
        d->m_pOptional_IdCardBox->setEnabled(!state);

        if (d->m_pMust_IdCardBox->checkState() == Qt::Unchecked)
          d->m_pMust_IdCardComparisonBox->setChecked(false);        
    }else if(d->m_pMust_IdCardComparisonBox == sender())
    {
        //人证比对是指,刷脸,身份证比对
        //要在人员注册时录入身份证,才能进行比对
        if(state) 
        {
            d->m_pMust_SwipingFaceBox->setChecked(true);
            d->m_pMust_IdCardBox->setChecked(true);
        }

    } 
}

void PassageModeFrm::slotSaveButton()
{
    Q_D(PassageModeFrm);

//    if(!this->getOpenIsEmpty())
    {
        QString mustOpenMode;
        if(d->m_pMust_SwipingicCardBox->isChecked())mustOpenMode.append("1&");
        if(d->m_pMust_SwipingFaceBox->isChecked())mustOpenMode.append("2&");
        if(d->m_pMust_ThermometryBox->isChecked())mustOpenMode.append("3&");
        if(d->m_pMust_MaskBox->isChecked())mustOpenMode.append("4&");
        if(d->m_pMust_QRcodeBox->isChecked())mustOpenMode.append("5&");
        if(d->m_pMust_IdCardBox->isChecked())mustOpenMode.append("6&");
        if(d->m_pMust_IdCardComparisonBox->isChecked())mustOpenMode.append("7");
        if(mustOpenMode.endsWith("&"))mustOpenMode = mustOpenMode.left(mustOpenMode.size() - 1);

        QString optOpenMode;
        if(d->m_pOptional_SwipingicCardBox->isChecked())optOpenMode.append("1|");
        if(d->m_pOptional_SwipingFaceBox->isChecked())optOpenMode.append("2|");
        if(d->m_pOptional_ThermometryBox->isChecked())optOpenMode.append("3|");
        if(d->m_pOptional_MaskBox->isChecked())optOpenMode.append("4|");
        if(d->m_pOptional_QRcodeBox->isChecked())optOpenMode.append("5|");
        if(d->m_pOptional_IdCardBox->isChecked())optOpenMode.append("6");
        if(optOpenMode.endsWith("|"))optOpenMode = optOpenMode.left(optOpenMode.size() - 1);


        if(mustOpenMode.size() < 1 && optOpenMode < 1)
        {
            OperationTipsFrm dlg(this);
            //dlg.setMessageBox(QObject::tr("温馨提示"), QObject::tr("必须和可选满足项不允许都为空!!!"));
            dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("requiredAndOptionalNotNULL"));
            return;
        }

        ReadConfig::GetInstance()->setDoor_MustOpenMode(mustOpenMode);
        ReadConfig::GetInstance()->setDoor_OptionalOpenMode(optOpenMode);
    }
//    else
//    {
//        OperationTipsFrm dlg(this);
//        dlg.setMessageBox(QObject::tr("温馨提示"), QObject::tr("必须满足项不允许为空!!!"));
//    }
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void PassageModeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 