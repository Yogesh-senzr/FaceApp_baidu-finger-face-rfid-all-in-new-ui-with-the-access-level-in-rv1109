#include "DoorLockFrm.h"
#include "Config/ReadConfig.h"
#include "PCIcore/Utils_Door.h"
#include "PCIcore/RkUtils.h"
#include "SharedInclude/GlobalDef.h"
#include "ManageEngines/PersonRecordToDB.h"
#include "Application/FaceApp.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "SettingFuncFrms/ManagingPeopleFrms/CameraPicFrm.h"


#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

class DoorLockFrmPrivate
{
    Q_DECLARE_PUBLIC(DoorLockFrm)
public:
    DoorLockFrmPrivate(DoorLockFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:

    QPushButton *m_pPwd1;
    QPushButton *m_pPwd2;
    QPushButton *m_pPwd3;

    QPushButton *m_pNum1;
    QPushButton *m_pNum2;
    QPushButton *m_pNum3;    

    QPushButton *m_pNum4;
    QPushButton *m_pNum5;
    QPushButton *m_pNum6;    

    QPushButton *m_pNum7;
    QPushButton *m_pNum8;
    QPushButton *m_pNum9;    

    QPushButton *m_pNum0;
    QPushButton *m_pDel;
    QTimer *m_pTimer;    

private:
    DoorLockFrm *const q_ptr;
};

DoorLockFrmPrivate::DoorLockFrmPrivate(DoorLockFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

DoorLockFrm::DoorLockFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new DoorLockFrmPrivate(this))
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

DoorLockFrm::~DoorLockFrm()
{

}

void DoorLockFrmPrivate::InitUI()
{ 
    m_pPwd1 = new QPushButton;
    m_pPwd2 = new QPushButton;
    m_pPwd3 = new QPushButton;

    m_pNum1 = new QPushButton;
    m_pNum2 = new QPushButton;
    m_pNum3 = new QPushButton;  

    m_pNum4 = new QPushButton;
    m_pNum5 = new QPushButton;
    m_pNum6 = new QPushButton;

    m_pNum7 = new QPushButton;
    m_pNum8 = new QPushButton;
    m_pNum9 = new QPushButton;

    m_pNum0 = new QPushButton;
    m_pDel =  new QPushButton;

    m_pTimer = new QTimer();
    m_pTimer->start(5000); // 5秒单触发定时器


    QHBoxLayout *layout = new QHBoxLayout;    
    layout->addWidget(m_pPwd1);
    layout->addWidget(m_pPwd2);
    layout->addWidget(m_pPwd3);

    QHBoxLayout *layout1 = new QHBoxLayout;    
    layout1->addWidget(m_pNum1);
    layout1->addWidget(m_pNum2);
    layout1->addWidget(m_pNum3);
    QHBoxLayout *layout2 = new QHBoxLayout;    
    layout2->addWidget(m_pNum4);
    layout2->addWidget(m_pNum5);
    layout2->addWidget(m_pNum6);    
    QHBoxLayout *layout3 = new QHBoxLayout;    
    layout3->addWidget(m_pNum7);
    layout3->addWidget(m_pNum8);
    layout3->addWidget(m_pNum9);      
    QHBoxLayout *layout4 = new QHBoxLayout;    
    layout4->addWidget(m_pNum0);
    layout4->addWidget(m_pDel);      

    QVBoxLayout *vLayout1 = new QVBoxLayout(q_func());
    vLayout1->setMargin(10);
    vLayout1->addLayout(layout);
    vLayout1->addLayout(layout1);
    vLayout1->addLayout(layout2);
    vLayout1->addLayout(layout3);  
    vLayout1->addLayout(layout4);     
}

void DoorLockFrmPrivate::InitData()
{
     /* 获取当前显示界面的长宽，以决定数字键盘的长宽 */
    unsigned int screenWidth = QApplication::desktop()->screenGeometry().width();
    unsigned int screenHeight = QApplication::desktop()->screenGeometry().height();

    /* 计算一些要素的尺寸 */
    unsigned int keyboardWidth;
    unsigned int keyboardHeight;
    // unsigned int boxWidth, boxHeight;
    // unsigned int horizonPadding, buttomPadding;
    // unsigned int buttonHeight;
    // unsigned int exitButtonHeight;
    // unsigned int titleHeight;
    if(screenWidth <= 480){
        keyboardWidth = screenWidth * 3 / 5;
    }else{
        keyboardWidth = screenWidth * 3 / 5;
    }
    
    keyboardHeight = screenHeight / 2;
    if (keyboardHeight > keyboardWidth)
        keyboardHeight = keyboardWidth;
    
    //调整大小
    q_func()->setFixedSize(keyboardWidth, keyboardHeight);
    //去掉背景
    q_func()->setStyleSheet("background: transparent;");

    m_pPwd1->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pPwd2->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pPwd3->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");

    m_pNum1->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum2->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum3->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");    
    m_pNum1->setText("1");
    m_pNum2->setText("2");
    m_pNum3->setText("3");

    m_pNum4->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum5->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum6->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");    
    m_pNum4->setText("4");
    m_pNum5->setText("5");
    m_pNum6->setText("6");

    m_pNum7->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum8->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum9->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");    
    m_pNum7->setText("7");
    m_pNum8->setText("8");
    m_pNum9->setText("9");


    m_pNum0->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pDel->setStyleSheet("background: rgba(0, 0, 0, 0.3);color: white;");
    m_pNum0->setText("0");
    m_pDel->setText("Del");

    m_pNum1->setProperty("btnNum", true);
    m_pNum2->setProperty("btnNum", true);
    m_pNum3->setProperty("btnNum", true);

    m_pNum4->setProperty("btnNum", true);
    m_pNum5->setProperty("btnNum", true);
    m_pNum6->setProperty("btnNum", true);

    m_pNum7->setProperty("btnNum", true);
    m_pNum8->setProperty("btnNum", true);
    m_pNum9->setProperty("btnNum", true);

    m_pNum0->setProperty("btnNum", true);    


    // m_pNum1->setFont(QFont("宋体", 22));//18,24
    // QFont blodFont = m_pNum1->font();
    // blodFont.setBold(true);
    // m_pNum1->setFont(blodFont);
    // m_pNum2->setFont(blodFont);
    // m_pNum3->setFont(blodFont);

    // m_pNum4->setFont(blodFont);
    // m_pNum5->setFont(blodFont);
    // m_pNum6->setFont(blodFont);

    // m_pNum7->setFont(blodFont);
    // m_pNum8->setFont(blodFont);
    // m_pNum9->setFont(blodFont);

    // m_pNum0->setFont(blodFont);
    // m_pDel->setFont(blodFont);


    m_pNum1->setObjectName("btn1");
    m_pNum2->setObjectName("btn2");    
    m_pNum3->setObjectName("btn3");

    m_pNum4->setObjectName("btn4");
    m_pNum5->setObjectName("btn5");    
    m_pNum6->setObjectName("btn6");

    m_pNum7->setObjectName("btn7");
    m_pNum8->setObjectName("btn8");    
    m_pNum9->setObjectName("btn9");    

    m_pNum0->setObjectName("btn0");     
    m_pDel->setObjectName("del");


}

void DoorLockFrmPrivate::InitConnect()
{

    QList<QPushButton *> btn = q_func()->findChildren<QPushButton *>();
    foreach (QPushButton * b, btn)
    {
        QObject::connect(b, SIGNAL(clicked()), q_func(), SLOT(btn_clicked()));
    }

    QObject::connect(m_pTimer, &QTimer::timeout, [&](){
        q_func()->close();
        });  
}





void DoorLockFrm::showEvent(QShowEvent *event)
{
    Q_D(DoorLockFrm);
  
    QDialog::showEvent(event);
}

void DoorLockFrm::setInputValue(int index)
{
    Q_D(DoorLockFrm);    

    //m_pPwdInputValue
    if (m_pPwdInputValue.length()<3)
    {
        m_pPwdInputValue=m_pPwdInputValue+QString::number(index);
    }
    if (m_pPwdInputValue.length()==1)
    {
        d->m_pPwd1->setText("*");    
    }
    else if (m_pPwdInputValue.length()==2)
    {
        d->m_pPwd2->setText("*");
    }
    else if (m_pPwdInputValue.length()==3)
    {
        d->m_pPwd3->setText("*");
        //开门        
        QString m_pPwd= ReadConfig::GetInstance()->getDoor_Password();

        if (m_pPwdInputValue == m_pPwd)
        {
            YNH_LJX::Utils_Door::GetInstance()->OpenDoor("");

            IdentifyFaceRecord_t t = { 0 };
            //t.process_state.append("&1");
            t.FaceType =0;// NOT_STRANGER;    //是否陌生人, 1是陌生人，2注册人员， 0未识别

           QString path = QString("/mnt/user/face_crop_image/%1/Door_%2.jpg").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"))
                    .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));                

            ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->saveCurFaceImg(path,90);

            t.FaceImgPath = path;
            t.FaceFullImgPath =path;
            //PersonRecordToDB::GetInstance()->appRecordData(t);					            
            PersonRecordToDB::GetInstance()->appDoorRecordData(0, path);

            close();
        }
        d->m_pPwd1->setText("");
        d->m_pPwd2->setText("");
        d->m_pPwd3->setText("");
        m_pPwdInputValue="";

    }

}

void DoorLockFrm::btn_clicked()
{
    Q_D(DoorLockFrm);       
    QPushButton *btn = (QPushButton *)sender();
    QString objectName = btn->objectName();
    if (btn->property("btnNum").toBool()) 
    {
        if (objectName == "btn0") {
            setInputValue(0);
        } else if (objectName == "btn1") {
            setInputValue(1);
        } else if (objectName == "btn2") {
            setInputValue(2);
        } else if (objectName == "btn3") {
            setInputValue(3);
        } else if (objectName == "btn4") {
            setInputValue(4);
        } else if (objectName == "btn5") {
            setInputValue(5);
        } else if (objectName == "btn6") {
            setInputValue(6);
        } else if (objectName == "btn7") {
            setInputValue(7);
        } else if (objectName == "btn8") {
            setInputValue(8);
        } else if (objectName == "btn9") {
            setInputValue(9);
        }
    } 
    if (objectName == "del")
    {
        if (m_pPwdInputValue.length()==1)
        {
            d->m_pPwd1->setText("");    
        }
        else if (m_pPwdInputValue.length()==2)
        {
            d->m_pPwd2->setText("");
        }
        else if (m_pPwdInputValue.length()==3)
        {
            d->m_pPwd3->setText("");
        }        
        m_pPwdInputValue=m_pPwdInputValue.mid(0, m_pPwdInputValue.length()-1);      
    }
    d->m_pTimer->stop();
    d->m_pTimer->start(10000);//5秒,10秒
    
}


void DoorLockFrm::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0)); /* 设置透明颜色 */
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void DoorLockFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 