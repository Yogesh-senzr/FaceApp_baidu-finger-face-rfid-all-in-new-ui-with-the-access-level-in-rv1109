#include "AddUserFrm.h"

#include "CameraPicFrm.h"
#include "SettingFuncFrms/SettingMenuTitleFrm.h"

#include "BaiduFace/BaiduFaceManager.h"
#include "OperationTipsFrm.h"

#include "DB/RegisteredFacesDB.h"
#include "Application/FaceApp.h"

#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"
#include "UserViewFrm.h"
#include "HttpServer/ConnHttpServerThread.h"


#include <QPropertyAnimation>
#include <QScreen>
#include <QApplication>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QApplication>
#include <QTransform>
#include <QSettings>
#include <QMessageBox>

static PERSONS_t mPerson = {0};
class AddUserFrmPrivate
{
    Q_DECLARE_PUBLIC(AddUserFrm)
public:
    AddUserFrmPrivate(AddUserFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    CameraPicFrm *m_pCameraPicFrm;//实时显示相机的图片
private:
    SettingMenuTitleFrm *m_pSettingMenuTitleFrm;
private:
    QLineEdit *m_pNameEdit;//姓名
    QLineEdit *m_pIDCardEdit;//身份证
    QLineEdit *m_pCardEdit;//卡号
    QComboBox *m_psexBox;//性别
    QPushButton *m_pCaptureFaceButton;//采集人脸
    QPushButton *m_pRegFaceButton;//注册人脸
private:
    QByteArray mFaceFeature;
private:
    QString mHintText;
private:
    AddUserFrm *const q_ptr;
};

AddUserFrmPrivate::AddUserFrmPrivate(AddUserFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

AddUserFrm::AddUserFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new AddUserFrmPrivate(this))
{
    d_func()->m_pCameraPicFrm = new CameraPicFrm(this);//实时显示相机的图片
    d_func()->m_pCameraPicFrm->setFixedSize(300, 256);
    d_func()->m_pCameraPicFrm->hide();
    QObject::connect(this, &AddUserFrm::sigModifyUser, this, &AddUserFrm::slotModifyUser);
}

AddUserFrm::~AddUserFrm()
{

}

void AddUserFrmPrivate::InitUI()
{
    LanguageFrm::GetInstance()->UseLanguage(0); //加上才能翻译    
    m_pSettingMenuTitleFrm = new SettingMenuTitleFrm;

    m_pNameEdit = new QLineEdit;//姓名
    m_pIDCardEdit = new QLineEdit;//身份证
    m_pCardEdit = new QLineEdit;//卡号
    m_psexBox = new QComboBox;//性别
    m_pCaptureFaceButton = new QPushButton;
    m_pRegFaceButton = new QPushButton;

    {
        QLabel *StarLabel = new QLabel("<p><font color=\"red\">*</font></p>");
        StarLabel->setStyleSheet("background:transparent;");
        QHBoxLayout* StarLayout = new QHBoxLayout;
        StarLayout->addStretch();
        StarLayout->addWidget(StarLabel);
        StarLayout->setSpacing(0);
        StarLayout->setContentsMargins(0, 0, 3, 0);
        m_pNameEdit->setLayout(StarLayout);
        m_pNameEdit->setContextMenuPolicy(Qt::NoContextMenu);

        
        QLabel *idCardStar = new QLabel("<p><font color=\"red\">*</font></p>");  // Create new star label
        idCardStar->setStyleSheet("background:transparent;");
        QHBoxLayout* idCardLayout = new QHBoxLayout;
        idCardLayout->addStretch();
        idCardLayout->addWidget(idCardStar);
        idCardLayout->setSpacing(0);
        idCardLayout->setContentsMargins(3, 0, 3, 0);
        m_pIDCardEdit->setLayout(idCardLayout);
        m_pIDCardEdit->setContextMenuPolicy(Qt::NoContextMenu);
    
        m_pCardEdit->setContextMenuPolicy(Qt::NoContextMenu);   
    }

        m_pNameEdit->setFocusPolicy(Qt::StrongFocus);
        m_pIDCardEdit->setFocusPolicy(Qt::StrongFocus);
        m_pCardEdit->setFocusPolicy(Qt::StrongFocus);

        m_pNameEdit->setAttribute(Qt::WA_AcceptTouchEvents);
        m_pIDCardEdit->setAttribute(Qt::WA_AcceptTouchEvents);
        m_pCardEdit->setAttribute(Qt::WA_AcceptTouchEvents);

        m_pNameEdit->installEventFilter(q_func());
        m_pIDCardEdit->installEventFilter(q_func());
        m_pCardEdit->installEventFilter(q_func());

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addSpacing(30);
    hlayout1->addWidget(new QLabel(QObject::tr(" Name:")));//姓名
    hlayout1->addWidget(m_pNameEdit);
    hlayout1->addSpacing(30);
    ((QLabel *)hlayout1->itemAt(1)->widget())->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addSpacing(30);
    hlayout2->addWidget(new QLabel(QObject::tr("EmpID:")));//身份证
    hlayout2->addWidget(m_pIDCardEdit);
    hlayout2->addSpacing(30);
    ((QLabel *)hlayout2->itemAt(1)->widget())->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *hlayout3 = new QHBoxLayout;
    hlayout3->addSpacing(30);
    hlayout3->addWidget(new QLabel(QObject::tr(" CardNo:")));//卡号
    hlayout3->addWidget(m_pCardEdit);
    hlayout3->addSpacing(30);
    ((QLabel *)hlayout3->itemAt(1)->widget())->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *hlayout4 = new QHBoxLayout;
    hlayout4->addSpacing(30);
    hlayout4->addWidget(new QLabel(QObject::tr("Gender:")));//性别
    hlayout4->addWidget(m_psexBox);
    hlayout4->addStretch();
    hlayout4->addSpacing(30);
    ((QLabel *)hlayout4->itemAt(1)->widget())->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *hlayout5 = new QHBoxLayout;
    hlayout5->addStretch();
    hlayout5->addWidget(m_pCaptureFaceButton);
    hlayout5->addWidget(m_pRegFaceButton);
    hlayout5->addStretch();

    QFrame *f = new QFrame;
    f->setStyleSheet("QFrame{background-color:rgb(231, 231, 231);}");


    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 5, 0, 20);
    layout->addLayout(hlayout1);
    layout->addLayout(hlayout2);
    layout->addLayout(hlayout3);
    layout->addLayout(hlayout4);
    layout->addSpacing(15);
    layout->addLayout(hlayout5);
    f->setLayout(layout);

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->addWidget(m_pSettingMenuTitleFrm);
    malayout->addStretch();
    malayout->addWidget(f);
}

void AddUserFrmPrivate::InitData()
{
	q_func()->setObjectName("AddUserFrm");
	
    m_psexBox->addItems(QStringList()<<QObject::tr("male")<<QObject::tr("female"));//男,女
    m_psexBox->setFixedSize(300, 58);
    m_pCaptureFaceButton->setFixedSize(220, 62);
    m_pRegFaceButton->setFixedSize(220, 62);
    m_pCaptureFaceButton->setText(QObject::tr("AcquisitionFace"));//人脸采集
    m_pRegFaceButton->setText(QObject::tr("RegisterFace"));//人脸注册
    m_pRegFaceButton->setEnabled(false);

    m_pNameEdit->setPlaceholderText(QObject::tr("Required"));//必填
    m_pIDCardEdit->setPlaceholderText(QObject::tr("Enter the employeeID as per SammyApp"));//选填
    m_pCardEdit->setPlaceholderText(QObject::tr("Optional"));//选填
    m_pSettingMenuTitleFrm->setTitleText(QObject::tr("AddPerson"));//新增人员
}

void AddUserFrmPrivate::InitConnect()
{
    QObject::connect(m_pCaptureFaceButton, &QPushButton::clicked, q_func(), &AddUserFrm::slotCaptureFaceButton);
    QObject::connect(m_pRegFaceButton, &QPushButton::clicked, q_func(), &AddUserFrm::slotRegFaceButton);

    QObject::connect(m_pSettingMenuTitleFrm, &SettingMenuTitleFrm::sigReturnSuperiorClicked, q_func(), &AddUserFrm::slotReturnSuperiorClicked);

    QObject::connect(m_pNameEdit, &QLineEdit::textChanged, q_func(), &AddUserFrm::slotTextChanged);

    QObject::connect(UserViewFrm::GetInstance(), &UserViewFrm::sigPersonInfo,  q_func(), &AddUserFrm::slotPersonInfo);    
    QObject::connect(m_pNameEdit, &QLineEdit::textChanged, q_func(), &AddUserFrm::slotTextChanged);

}

void AddUserFrm::slotModifyUser()
{
    Q_D(AddUserFrm);
    mModify++;
    if (mModify<=1 && mPerson.name!=nullptr )
    {        
        d->m_pSettingMenuTitleFrm->setTitleText(QObject::tr("ModifyPerson"));	
        d->m_pNameEdit->setText(mPerson.name);
        d->m_pIDCardEdit->setText(mPerson.idcard);
        d->m_pCardEdit->setText(mPerson.iccard);//卡号
    }

}
void AddUserFrm::modifyRecord(QString aName,QString aIDCard,QString aCardNo,QString asex,QString apersonid,QString auuid)
{
    Q_D(AddUserFrm);

    d->m_pSettingMenuTitleFrm->setTitleText(QObject::tr("ModifyPerson"));
    //qDebug()<<__LINE__<<"name="<<aName<<",aIDCard="<<aIDCard<<",aCardNo="<<aCardNo<<",asex="<<asex<<",apersonid="<<apersonid<<",auuid="<<auuid;
    d->m_pNameEdit->setText(aName); //姓名
    d->m_pIDCardEdit->setText(aIDCard);//身份证
    d->m_pCardEdit->setText(aCardNo);//卡号
    //d->m_psexBox->itemText(asex);//性别    
    //return exec();

    mPerson.name = aName;
    mPerson.sex = asex; 
    mPerson.idcard= aIDCard;
    mPerson.iccard= aCardNo;
    mPerson.personid= apersonid.toInt();
    mPerson.uuid= auuid;
	
}


void AddUserFrm::slotCaptureFaceButton()
{
    Q_D(AddUserFrm);
    int faceNum = 0;
    double threshold = 0;
    int ret = -1;
    QString path;
    system("rm /mnt/user/facedb/RegImage.jpg");
    system("rm /mnt/user/facedb/RegImage.jpeg");
    path = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getCurFaceImgPath();
    if (path.length()==0)
    {
        if (QMessageBox::question(this, QObject::tr("Tips"), QObject::tr("Please activate the algorithm first!"),
            QMessageBox::Ok)== QMessageBox::Ok)
        {
            return ;
        }

    }
    printf("%s,%s,%d path=%s\n",__FILE__,__func__,__LINE__,path.toStdString().c_str());

    QImage img = QImage(path);///mnt/user/facedb/RegImage.jpeg
    mPath =path;
    mDraw = true;    
//如果主界面旋转,图片也跟着旋转   echo $QT_QPA_PLATFORM  -platform linuxfb:fb=/dev/fb0:rotation=180
    QSettings sysIniFile("/param/RV1109_PARAM.txt", QSettings::IniFormat);
    int rotation = sysIniFile.value("PLATFORM_ROTATION",  0).toInt();
    printf(">>>>%s,%s,%d,rotation=%d\n",__FILE__,__func__,__LINE__,rotation);
    if (rotation>0)
    {
        QTransform transform;
        transform.rotate(rotation);
        img = img.transformed(transform);
    }
    d->m_pCameraPicFrm->setShowImage(img);
    d->m_pCameraPicFrm->setShowImage(img);
    d->m_pCameraPicFrm->move(this->width() - 175, this->height() - 625);
    d->m_pCameraPicFrm->update();
    d->m_pCameraPicFrm->show();
    ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(path, faceNum, threshold, d->mFaceFeature);
    if(ret == -1)
    {
        d->mHintText = QObject::tr("ErrorDataAcquisiting");//采集人脸出错,请重新采集人脸 data acquisition Error in collecting face. Please collect face again.
    }
    else if(ret==0 && (faceNum == 0))
    {
        d->mHintText = QObject::tr("NotFindFace");//未发现人脸,请重新采集人脸!!! Did not find a face, please gather again face!!!!!!
    }else if(ret==0 && (faceNum != 1))
    {
        d->mHintText = QObject::tr("MultipleFaces");//有多张人脸,请重新采集人脸!!!There are multiple faces, please re-collect faces! ! !
    }else if(ret==0 && (threshold <= 0.7))
    {
    	d->mHintText = QString::number(threshold,'f',2) +QString(" ");
        d->mHintText += QObject::tr("LowFaceQuality");//人脸质量低,请重新采集人脸!!!,The face quality is low, please re-collect the face! ! !
    }else if(ret == 0 && faceNum == 1)
    {
        d->mHintText = QObject::tr("FaceDataAcquisitOk");//人脸采集OK!!!Collect faces ok! ! !
    }else d->mHintText.clear();
    QFile fileTemp(path);
    fileTemp.remove();

    if(d->mHintText != QObject::tr("FaceDataAcquisitOk") || d->m_pNameEdit->text().isEmpty() || 
    d->m_pIDCardEdit->text().isEmpty())//人脸采集OK!!!
        d->m_pRegFaceButton->setEnabled(false);
    else { d->m_pRegFaceButton->setEnabled(true);
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/000%1.png").arg(this->metaObject()->className()),"png");    
#endif     
    }

    this->update();
    
}




void AddUserFrm::slotRegFaceButton()
{
    Q_D(AddUserFrm);
    OperationTipsFrm dlg(this);

    // Determine if we're modifying an existing person
    bool isModifyingPerson = (d->m_pSettingMenuTitleFrm->getTitleText() == QObject::tr("ModifyPerson"));

    // Face duplicate check
    if(!d->mFaceFeature.isEmpty()) {
        QSqlQuery checkFaceQuery(QSqlDatabase::database("isc_arcsoft_face"));
        checkFaceQuery.prepare("SELECT personid, feature FROM person");
        
        if (!checkFaceQuery.exec()) {
            qDebug() << "Face query error:" << checkFaceQuery.lastError().text();
            dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("Database error occurred!"));
            return;
        }

        while(checkFaceQuery.next()) {
            int existingPersonId = checkFaceQuery.value("personid").toInt();
            QByteArray dbFeature = checkFaceQuery.value("feature").toByteArray();
            
            // If modifying, exclude the current person's record from duplicate check
            if (!dbFeature.isEmpty() && 
                (!isModifyingPerson || existingPersonId != mPerson.personid)) {
                
                double similarity = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare_baidu(
                    (unsigned char*)d->mFaceFeature.data(),
                    d->mFaceFeature.size(),
                    (unsigned char*)dbFeature.data(),
                    dbFeature.size()
                );
                
                if(similarity > 0.8) {
                    dlg.setMessageBox(QObject::tr("Tips"), 
                        QObject::tr("Face already exists! Similarity: %1%").arg(similarity * 100));
                    return;
                }
            }
        }
    }

    // ID Card duplicate check
    QString idCardNum = d->m_pIDCardEdit->text();
    QSqlQuery checkQuery(QSqlDatabase::database("isc_arcsoft_face"));
    checkQuery.prepare("SELECT personid FROM person WHERE idcardnum = :idcard");
    checkQuery.bindValue(":idcard", idCardNum);
    
    if (!checkQuery.exec()) {
        qDebug() << "Failed to query database for duplicate ID card:" << checkQuery.lastError().text();
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("Database error occurred!"));
        return;
    }
    
    while (checkQuery.next()) {
        int existingPersonId = checkQuery.value("personid").toInt();
        
        // If modifying, allow keeping the same ID card for the current person
        if (!isModifyingPerson || existingPersonId != mPerson.personid) {
            qDebug() << "Duplicate ID card found:" << idCardNum;
            dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("ID Card already exists in database!"));
            return;
        }
    }

  // Card number duplicate check
QString cardNo = d->m_pCardEdit->text().trimmed(); // Trim any whitespace
if (!cardNo.isEmpty() && cardNo != "000000") // Add additional check for meaningful card number
{
    QSqlQuery checkCardQuery(QSqlDatabase::database("isc_arcsoft_face"));
    checkCardQuery.prepare("SELECT personid FROM person WHERE iccardnum = :cardno AND iccardnum != '000000' AND iccardnum != ''");
    checkCardQuery.bindValue(":cardno", cardNo);
    
    if (!checkCardQuery.exec()) {
        qDebug() << "Failed to query database for duplicate card number:" << checkQuery.lastError().text();
        dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("Database error occurred!"));
        return;
    }
    
    while (checkCardQuery.next()) {
        int existingPersonId = checkCardQuery.value("personid").toInt();
        
        // If modifying, allow keeping the same card number
        // Exclude default/empty card numbers from duplicate checks
        if (!isModifyingPerson || existingPersonId != mPerson.personid) {
            qDebug() << "Duplicate card number found:" << cardNo;
            dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("Card number already exists in database!"));
            return;
        }
    }
}
    // Proceed with registration
    qDebug() << "REG_FACE_DEBUG: All checks passed, proceeding with registration";
    d->m_pRegFaceButton->setEnabled(false);
    d->m_pCameraPicFrm->setShowImage(QImage());
    d->m_pCameraPicFrm->hide();
    
    int ret = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
        "", 
        d->m_pNameEdit->text(), 
        d->m_pIDCardEdit->text(), 
        d->m_pCardEdit->text(), 
        d->m_psexBox->currentText(),
        "", 
        "",
        d->mFaceFeature
    );
    
    qDebug() << "REG_FACE_DEBUG: RegPersonToDBAndRAM returned: " << ret;
    
    dlg.setMessageBox(
        QObject::tr("Tips"), 
        QObject::tr("Register【%1】Person%2!!!").arg(d->m_pNameEdit->text()).arg(ret ? QObject::tr("Succes") : QObject::tr("Fail"))
    );

    if (ret == true) {
        qDebug() << "REG_FACE_DEBUG: Registration successful, querying new UUID";
        
        // Get the UUID that was just created
        QString newUuid;
        QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
        query.exec("SELECT uuid FROM person ORDER BY personid DESC LIMIT 1");
        if (query.next()) {
            newUuid = query.value("uuid").toString();
            qDebug() << "REG_FACE_DEBUG: New UUID: " << newUuid;
        }
        else {
            qDebug() << "REG_FACE_DEBUG: Failed to retrieve new UUID!";
        }
        
        qDebug() << "REG_FACE_DEBUG: Calling sendUserToServer...";
        bool sendResult = ConnHttpServerThread::GetInstance()->sendUserToServer(
            newUuid,
            d->m_pNameEdit->text(),
            d->m_pIDCardEdit->text(),
            d->m_pCardEdit->text(),
            d->m_psexBox->currentText(),
            "",  // This should be a QString, not a char array
            "",  // timeOfAccess - should be passed as a QString
            d->mFaceFeature
        );
        
        qDebug() << "REG_FACE_DEBUG: sendUserToServer returned: " << sendResult;
    }
    else {
        qDebug() << "REG_FACE_DEBUG: Registration failed!";
    }

    // For modifying an existing person, remove the old record
    if (ret == true && isModifyingPerson)
    {
        QString cmdline = "sqlite3 /mnt/user/facedb/isc_arcsoft_face.db 'delete from person where personid=" + 
            QString::number(mPerson.personid) + 
            QString(" and uuid=") + "\"" + mPerson.uuid + "\" ';";
        system(cmdline.toStdString().data());
    }
    
    d->mHintText.clear();
}

void AddUserFrm::slotReturnSuperiorClicked()
{
    Q_D(AddUserFrm);
    d->m_pCameraPicFrm->setShowImage(QImage());
    d->m_pCameraPicFrm->hide();
    
    if (d->m_pSettingMenuTitleFrm->getTitleText()==QObject::tr("ModifyPerson"))
    {
        d->m_pSettingMenuTitleFrm->setTitleText(QObject::tr("AddPerson"));
        //emit sigShowFaceHomeFrm(0);
    }



    emit sigShowFaceHomeFrm();

    UserViewFrm::GetInstance()->setModifyFlag(0);    
    
    d->m_pNameEdit->clear();
    d->m_pIDCardEdit->clear();
    d->m_pCardEdit->clear();//卡号    		
    mPerson = {0};
    mModify= 0;
}

void AddUserFrm::slotTextChanged(const QString &text)
{
    Q_D(AddUserFrm);
    if(text.isEmpty() || d->m_pCameraPicFrm->getImgisNull())
    {
        d->m_pRegFaceButton->setEnabled(false);
    }else if(!text.isEmpty() && !d->m_pCameraPicFrm->getImgisNull() &&  d->mHintText == QObject::tr("FaceDataAcquisitOk"))//人脸采集OK!!!
    {
        d->m_pRegFaceButton->setEnabled(true);
    }
}

void AddUserFrm::slotPersonInfo(const QString &aName ,const QString &asex ,const QString &aCard,const QString &cardNo)
{
    Q_D(AddUserFrm);

    mPerson.name = aName;
    mPerson.sex = asex; 
    mPerson.idcard= aCard;
    mPerson.iccard= cardNo;
}
void AddUserFrm::hideEvent(QHideEvent *event)
{
    Q_D(AddUserFrm);
    d->mHintText.clear();
    d->m_pRegFaceButton->setEnabled(false);
    QWidget::hideEvent(event);
}

void AddUserFrm::paintEvent(QPaintEvent *event)
{
    Q_D(AddUserFrm);
    QFont fnt = this->font();
    fnt.setPixelSize(30);
    QFontMetrics fm(fnt);
    int fmw = fm.width(d->mHintText);

    QPainter painter(this);
    painter.setPen(Qt::red);
    painter.setFont(fnt);
    painter.drawText((this->width()/2) - (fmw/2), 180, d->mHintText);
    QWidget::paintEvent(event);

    emit sigModifyUser(); 

#ifdef SCREENCAPTURE  //ScreenCapture 
    printf(">>>>%s,%s,%d,mPath=%s, mDraw=%d\n",__FILE__,__func__,__LINE__, mPath.toStdString().c_str(), mDraw); 
    if (!mPath.isEmpty() && mDraw)
    {
        mDraw = false;
        Q_UNUSED(event);  
        QPainter painter2(this);  
        QPixmap pix;
        pix.load(mPath);
        painter2.drawPixmap(0,0,800,1280,pix);//0,0,600,800 0,0,800,1280
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);        
        grab().save(QString("/mnt/user/screenshot/painterAddUserFrm.png"),"png"); 
    }
#endif     
}

bool AddUserFrm::eventFilter(QObject *obj, QEvent *event)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(obj);
    if (lineEdit) {
        if (event->type() == QEvent::MouseButtonPress) {
            // Get the parent frame
            QFrame *formFrame = findChild<QFrame*>();
            
            // Move form up and show keyboard immediately
            if (!formFrame->property("isMovedUp").toBool()) {
                // Calculate new position
                QRect startGeometry = formFrame->geometry();
                int keyboardHeight = 600;
                
                QPoint globalPos = lineEdit->mapToGlobal(lineEdit->rect().bottomLeft());
                int screenHeight = QApplication::primaryScreen()->geometry().height();
                int offset = 0;
                
                if (globalPos.y() > (screenHeight - keyboardHeight - 50)) {
                    offset = globalPos.y() - (screenHeight - keyboardHeight - 50);
                }

                // Create animation
                QPropertyAnimation *animation = new QPropertyAnimation(formFrame, "geometry");
                animation->setDuration(300);
                
                QRect endGeometry = startGeometry;
                endGeometry.moveTop(startGeometry.y() - offset);
                
                animation->setStartValue(startGeometry);
                animation->setEndValue(endGeometry);
                animation->setEasingCurve(QEasingCurve::OutCubic);
                
                // Store original position
                formFrame->setProperty("originalY", startGeometry.y());
                formFrame->setProperty("isMovedUp", true);
                
                // Start animation and immediately show keyboard
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                
                // Force focus and show keyboard
                lineEdit->setFocus();
                
                // For embedded Linux, we can try to force show the virtual keyboard
                system("killall -9 keyboard");
                system("/usr/bin/keyboard &");
            }
            
            return true; // Handle the event
        }
        else if (event->type() == QEvent::FocusOut) {
            QFrame *formFrame = findChild<QFrame*>();
            if (formFrame && formFrame->property("isMovedUp").toBool()) {
                // Move form back down
                QPropertyAnimation *animation = new QPropertyAnimation(formFrame, "geometry");
                animation->setDuration(300);
                
                QRect currentGeometry = formFrame->geometry();
                QRect endGeometry = currentGeometry;
                endGeometry.moveTop(formFrame->property("originalY").toInt());
                
                animation->setStartValue(currentGeometry);
                animation->setEndValue(endGeometry);
                animation->setEasingCurve(QEasingCurve::OutCubic);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                
                formFrame->setProperty("isMovedUp", false);
                
                // Hide keyboard
                system("killall -9 keyboard");
            }
        }
    }
    
    return QWidget::eventFilter(obj, event);
}
#ifdef SCREENCAPTURE  //ScreenCapture
void AddUserFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__); 
    //grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
    mPath = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getCurFaceImgPath();
    mDraw = true;
    if (!mPath.isEmpty() && mDraw)
    {
       // mDraw = false;
        Q_UNUSED(event);  
        QPainter painter(this);  
        QPixmap pix;
        pix.load(mPath);
        painter.drawPixmap(0,0,800,1280,pix);//0,0,600,800 0,0,800,1280
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);        
        grab().save(QString("/mnt/user/screenshot/painterAddUserFrm.png"),"png"); 
    }    
}	
#endif 
