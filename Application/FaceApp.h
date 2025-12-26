#ifndef FaceApp_H
#define FaceApp_H

#include <QtWidgets/QApplication>
#include <QScreen>
#include <QtWidgets/QDesktopWidget>

#ifndef qXLApp
#define qXLApp (static_cast<FaceApp *>(QCoreApplication::instance()))
#endif

#ifndef TC_FREE1
#define TC_FREE1(x)  {delete []x; x=Q_NULLPTR;}
#endif

#ifndef DPIVal
#define DPIVal (QApplication::primaryScreen()->physicalDotsPerInch())
#endif

#ifndef _QWidgetAppStyle_
#define _QWidgetAppStyle_

#define QWidgetAppStyle(widget)  do {\
    if(DeskTopWidth>=800 && DeskTopHeight<=1280)\
        widget->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle800×1280.css"));\
    else if(DeskTopWidth>=720 && DeskTopHeight<=1280)\
        widget->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle720×1280.css"));\
    else if(DeskTopWidth>=600 && DeskTopHeight<=1024)\
        widget->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle600×1024.css"));\
	}while(0);

#endif


#ifndef DeskTopWidth
#define DeskTopWidth (QApplication::desktop()->screenGeometry().width())
#endif

#ifndef DeskTopHeight
#define DeskTopHeight (QApplication::desktop()->screenGeometry().height())
#endif

class powerManagerThread;
class IdentityCardManage;
class IdentityCard_ZK_Manage;
class HealthCodeManage;
class RecordsExport;
class PostPersonRecordThread;
class ParsePersonXlsx;
class IdentityManagement;
class SensorManager;
//class ArcsoftFaceManager;
class BaseFaceManager;
class CameraManager;
class FingerprintManager;
class FaceMainFrm;
class FaceAppPrivate;
class FaceApp : public QApplication
{
    Q_OBJECT
public:
    FaceApp(int &argc, char ** argv);
    virtual ~FaceApp();
public:
    static inline FaceApp *instance() { return qobject_cast<FaceApp *>(QCoreApplication::instance()); }
    static void connectReady(QObject *receiver, const char *member, Qt::ConnectionType type = Qt::AutoConnection);
public:
    static void Sleep(int sec);
public:   
    static FaceMainFrm *GetFaceMainFrm();
public:
    static CameraManager *GetCameraManager();
    //static ArcsoftFaceManager *GetAlgoFaceManager();
    static BaseFaceManager *GetAlgoFaceManager(); //BaseFaceManager BaiduFaceManager
    static SensorManager *GetSensorManager();
    static IdentityManagement *GetIdentityManagement();
    static ParsePersonXlsx *GetParsePersonXlsx();
    static RecordsExport *GetRecordsExport();
    static PostPersonRecordThread *GetPostPersonRecordThread();
    static powerManagerThread *GetPowerManagerThread();
    static HealthCodeManage *GetHealthCodeManage();
    static IdentityCardManage *GetIdentityCardManage();
    static IdentityCard_ZK_Manage *GetIdentityCard_ZK_Manage();
    static FingerprintManager *GetFingerprintManager();
private:
    Q_SIGNAL void ready();
private:
    QScopedPointer<FaceAppPrivate>d_ptr;
private:
    Q_DISABLE_COPY(FaceApp)
    Q_DECLARE_PRIVATE(FaceApp)
};

#endif
