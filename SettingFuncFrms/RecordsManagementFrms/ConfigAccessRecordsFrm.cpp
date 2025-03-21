#include "ConfigAccessRecordsFrm.h"

#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class ConfigAccessRecordsFrmPrivate
{
    Q_DECLARE_PUBLIC(ConfigAccessRecordsFrm)
public:
    ConfigAccessRecordsFrmPrivate(ConfigAccessRecordsFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QCheckBox *m_pSaveStrangerRecordBox;//保存陌生人记录
    QCheckBox *m_pSaveFaceBox;//保存人脸
    QCheckBox *m_pSaveFullShotBox;//保存全景

    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    ConfigAccessRecordsFrm *const q_ptr;
};

ConfigAccessRecordsFrmPrivate::ConfigAccessRecordsFrmPrivate(ConfigAccessRecordsFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ConfigAccessRecordsFrm::ConfigAccessRecordsFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new ConfigAccessRecordsFrmPrivate(this))
{

}

ConfigAccessRecordsFrm::~ConfigAccessRecordsFrm()
{

}

void ConfigAccessRecordsFrmPrivate::InitUI()
{    
    LanguageFrm::GetInstance()->UseLanguage(0);//从配置中读取

    m_pSaveStrangerRecordBox = new QCheckBox;//保存陌生人记录
    m_pSaveFaceBox = new QCheckBox;//保存人脸
    m_pSaveFullShotBox = new QCheckBox;
    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("settingPassRecord")));//设置通行记录
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(10);
    layout1->addWidget(new QLabel(QObject::tr("SaveFacePicture")));//保存人脸图片
    layout1->addStretch();
    layout1->addWidget(m_pSaveFaceBox);
    layout1->addSpacing(10);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(10);
    layout2->addWidget(new QLabel(QObject::tr("SaveNewPersonRecord")));//保存陌生人记录
    layout2->addStretch();
    layout2->addWidget(m_pSaveStrangerRecordBox);
    layout2->addSpacing(10);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addSpacing(10);
    layout3->addWidget(new QLabel(QObject::tr("SaveFullPicture")));//保存全景图片
    layout3->addStretch();
    layout3->addWidget(m_pSaveFullShotBox);
    layout3->addSpacing(10);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    layout1->itemAt(1)->widget()->setObjectName("DialogLabel");
    layout2->itemAt(1)->widget()->setObjectName("DialogLabel");
    layout3->itemAt(1)->widget()->setObjectName("DialogLabel");


    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->addSpacing(10);
    layout4->addWidget(m_pConfirmButton);
    layout4->addWidget(m_pCancelButton);
    layout4->addSpacing(10);

    QVBoxLayout *vlayout= new QVBoxLayout(q_func());
    vlayout->setContentsMargins(0, 5, 0, 15);
    vlayout->addLayout(layout);
    vlayout->addLayout(layout3);
    vlayout->addLayout(layout1);
    vlayout->addLayout(layout2);
    vlayout->addSpacing(10);
    vlayout->addLayout(layout4);
}

void ConfigAccessRecordsFrmPrivate::InitData()
{
    q_func()->setObjectName("ConfigAccessRecordsFrm");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void ConfigAccessRecordsFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {q_func()->done(0); });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void ConfigAccessRecordsFrm::setInitConfig(const bool state, const bool state1, const bool state2)
{
    Q_D(ConfigAccessRecordsFrm);

    d->m_pSaveFullShotBox->setChecked(state);
    d->m_pSaveFaceBox->setChecked(state1);//保存人脸
    d->m_pSaveStrangerRecordBox->setChecked(state2);//保存陌生人记录
}

bool ConfigAccessRecordsFrm::getPanoramaState() const
{
    return d_func()->m_pSaveFullShotBox->checkState();
}

bool ConfigAccessRecordsFrm::getFaceState() const
{
    return d_func()->m_pSaveFaceBox->checkState();
}

bool ConfigAccessRecordsFrm::getStrangerState() const
{
    return d_func()->m_pSaveStrangerRecordBox->checkState();
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void ConfigAccessRecordsFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif