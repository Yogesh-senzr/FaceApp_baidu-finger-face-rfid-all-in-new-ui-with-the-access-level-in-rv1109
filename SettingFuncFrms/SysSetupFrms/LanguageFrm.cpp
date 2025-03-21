#include "LanguageFrm.h"
#include "Config/ReadConfig.h"
#include "OperationTipsFrm.h"


#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFile>
#include <QApplication>
#include <QMouseEvent>
#include <QTranslator>
#include <QMessageBox>


LanguageFrm* LanguageFrm::LangInst = nullptr;

class LanguageFrmPrivate
{
    Q_DECLARE_PUBLIC(LanguageFrm)
public:
    LanguageFrmPrivate(LanguageFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pLanguageGroup;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    LanguageFrm *const q_ptr;
    QTranslator * m_trans;    
};

LanguageFrmPrivate::LanguageFrmPrivate(LanguageFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

LanguageFrm::LanguageFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LanguageFrmPrivate(this))
{

}

LanguageFrm::~LanguageFrm()
{

}
LanguageFrm* LanguageFrm::GetLanguageInstance()
{
    if (nullptr == LangInst)
    {
        LangInst = new LanguageFrm;        
    }
    return LangInst;
}

void LanguageFrmPrivate::InitUI()
{
    m_pLanguageGroup = new QButtonGroup;
    m_pLanguageGroup->addButton(new QRadioButton, 0);//中文
    //m_pLanguageGroup->addButton(new QRadioButton, 1);//英语
    //m_pLanguageGroup->addButton(new QRadioButton, 2);//日语
    //m_pLanguageGroup->addButton(new QRadioButton, 3);//韩语

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("LanguageSettings")));//语言设置
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addSpacing(20);
    layout1->addWidget(new QLabel(QObject::tr("English")));
    layout1->addStretch();
    layout1->addWidget(m_pLanguageGroup->button(0));
    layout1->addSpacing(20);
#if 0

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addSpacing(20);
    layout2->addWidget(new QLabel(QObject::tr("中文")));
    layout2->addStretch();
    layout2->addWidget(m_pLanguageGroup->button(1));
    layout2->addSpacing(20);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addSpacing(20);
    layout3->addWidget(new QLabel(QObject::tr("日本語")));
    layout3->addStretch();
    layout3->addWidget(m_pLanguageGroup->button(2));
    layout3->addSpacing(20);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->addSpacing(20);
    layout4->addWidget(new QLabel(QObject::tr("한글")));
    layout4->addStretch();
    layout4->addWidget(m_pLanguageGroup->button(3));
    layout4->addSpacing(20);
#endif 
    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->addSpacing(10);
    layout5->addWidget(m_pConfirmButton);
    layout5->addWidget(m_pCancelButton);
    layout5->addSpacing(10);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    layout1->itemAt(1)->widget()->setObjectName("DialogLabel");
    //layout2->itemAt(1)->widget()->setObjectName("DialogLabel");
    //layout3->itemAt(1)->widget()->setObjectName("DialogLabel");
    //layout4->itemAt(1)->widget()->setObjectName("DialogLabel");


    QVBoxLayout *vlayout= new QVBoxLayout(q_func());
    vlayout->setContentsMargins(0, 5, 0, 15);
    {
        vlayout->addLayout(layout);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
    {
        vlayout->addLayout(layout1);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
#if 0
    {
        vlayout->addLayout(layout2);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }    
    {
        vlayout->addLayout(layout3);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
    {
        vlayout->addLayout(layout4);
        QFrame *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("HLineFrm");
        vlayout->addWidget(f);
    }
#endif 

    vlayout->addLayout(layout5);

    m_trans = new QTranslator;   

}

void LanguageFrmPrivate::InitData()
{
    q_func()->setObjectName("LanguageFrm");

    m_pLanguageGroup->button(0)->setChecked(true);
    m_pLanguageGroup->button(0)->setObjectName("choiceRadioButton");
    //m_pLanguageGroup->button(1)->setObjectName("choiceRadioButton");
    //m_pLanguageGroup->button(2)->setObjectName("choiceRadioButton");
    //m_pLanguageGroup->button(3)->setObjectName("choiceRadioButton");

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消
}

void LanguageFrmPrivate::InitConnect()
{
    //QObject::connect(q_func(), &LanguageFrm::LanguageChaned, this,  &RetranslateUI);
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {
        int index=-1;        
#if 0        
        if (QMessageBox::question(q_func(), QObject::tr("Tips"), QObject::tr("ChangeLanguageHint"),
            QMessageBox::Yes|QMessageBox::No)== QMessageBox::No)
        {
            index =ReadConfig::GetInstance()->getLanguage_Mode();
            //ReadConfig::GetInstance()->setLanguage_Mode(index);
            m_pLanguageGroup->button(index)->setChecked(true);
            return;
        }  
#else
        OperationTipsFrm dlg(q_func());
        //系统升级,确定,请索取update.img升级包文件放入U盘(FAT32)，插入U盘即可自动升级!
        if (0 != dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("ChangeLanguageHint")))
        {
            index =ReadConfig::GetInstance()->getLanguage_Mode();
            m_pLanguageGroup->button(index)->setChecked(true);
            return;
        }     
#endif         
        //m_pLanguageGroup->button(0)->setEnabled(false);
        //m_pLanguageGroup->button(1)->setEnabled(false);
        q_func()->setVisible(false);

        if (m_pLanguageGroup->button(0)->isChecked())
           index =0;
        if (m_pLanguageGroup->button(1)->isChecked())
           index =1;

        ReadConfig::GetInstance()->setLanguage_Mode(index);
        ReadConfig::GetInstance()->setSaveConfig();
        system("reboot");
        //q_func()->setLanguageMode(index); 
        //q_func()->done(0); 
       
        });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });

    QObject::connect(m_pLanguageGroup,  static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index){
        QString strLanguageFile;
        QTranslator translator;
        switch(index)
        {
        case 0://中文
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
        case 1://英文
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
           
        default:
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
        }
        //printf(">>>%s,%s,%d,index=%d\n",__FILE__,__func__,__LINE__,index);          
        //m_pLanguageGroup->button(index)->setChecked(true);
        /*
        if (QFile(strLanguageFile).exists())
        {
            translator.load(strLanguageFile);
            qApp->installTranslator(&translator);
           // emit q_func()->LanguageChaned();//发出语言被切换的信号
        }
        */
    });
}

void LanguageFrm::setLanguageMode(const int &index)
{
    Q_D(LanguageFrm);
      
    d->m_pLanguageGroup->button(index)->setChecked(true);

}

void LanguageFrm::UseLanguage(int forceload)  //0: load, 1; 语言已更改,要 forceload
{
    Q_D(LanguageFrm);    
    if (forceload==0)
    {
       // if (nullptr != d->m_trans) //已经装载语言包
       //     return;
    }

    //1.从配置文件中读取 
    QString strLanguageFile;
    int index = ReadConfig::GetInstance()->getLanguage_Mode();

    switch(index)
    {
        case 0://中文
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
        case 1://英文
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;           
        default:
            strLanguageFile = QString("/isc/languages_baidu/innohi_en.qm");
            break;
    }
    if (QFile(strLanguageFile).exists())
    {
        if (nullptr != d->m_trans)
            qApp->removeTranslator(d->m_trans);     
        d->m_trans->load(strLanguageFile);
        qApp->installTranslator(d->m_trans);
        emit LanguageChaned();//发出语言被切换的信号 //ok
    }       

}

int LanguageFrm::getLanguageMode() const
{
    return d_func()->m_pLanguageGroup->checkedId();
}

void LanguageFrm::mousePressEvent(QMouseEvent *event)
{


#ifdef Q_OS_WIN
    if(event->pos().y()>=47 && event->pos().y()<=97)
        this->setLanguageMode(0);
    else if(event->pos().y()>=100.0 && event->pos().y()<150)
        this->setLanguageMode(1);
    else if(event->pos().y()>=150 && event->pos().y()<=200)
        this->setLanguageMode(2);
    else if(event->pos().y()>=204 && event->pos().y()<=250)
        this->setLanguageMode(3);
#else
    int index= -1;
    if(event->pos().y()>=68 && event->pos().y()<=128)
        index= 0;
    else if(event->pos().y()>=137 && event->pos().y()<200)
        index= 1;
    if (index<0 || index>1 ) index =0;    
    this->setLanguageMode(index);           
#endif
    QDialog::mousePressEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void LanguageFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 
