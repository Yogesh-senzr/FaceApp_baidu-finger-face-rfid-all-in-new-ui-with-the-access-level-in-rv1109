#ifndef FACEMAINFRM_H
#define FACEMAINFRM_H
#include "RKCamera/Camera/cameramanager.h"
#include <QWidget>
#include <QLabel>

class FaceMainFrmPrivate;
class FaceMainFrm : public QWidget
{
    Q_OBJECT
public:
	int getStatus()
	{
		return mstatus;
	}
	void setStatus(int c)
	{
		mstatus = c;
	}
    FaceMainFrm(QWidget *parent = 0);
    ~FaceMainFrm();
public:
    void setHomeDisplay_SnNum(const int &);
    void setHomeDisplay_Mac(const int &);
    void setHomeDisplay_IP(const int &);
    void setHomeDisplay_PersonNum(const int &);
    void setHomeDisplay_DoorLock(const int &);    
    void updateHome_PersonNum();
    void showThreadInfo();
        // Add these new sync-related methods
    void setTenantName(const QString &tenantName);
    void updateSyncStatus(const QString &status);
    void updateSyncUserCount(int currentCount, int totalCount);
    void updateLastSyncTime(const QString &time);
    void updateLocalFaceCount(int localCount, int totalCount);  // New method
    void triggerSettings();

public:
     //固件更新
    Q_SLOT void slotUpDateTip(const QString);
    //当前无人脸
    Q_SLOT void slotDisMissMessage(const bool state);
    /*参数：位置(顶与底), 序号(上下各4项)， 提示消息*/
    Q_SLOT void slotTipsMessage(const int, const int, const QString);
    //健康码（名称， 身份证号 健康码）
    Q_SLOT void slotHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);

    Q_SLOT void slotShowAlgoStateAboutFace(const QString);
        // ADD THESE TWO NEW SLOTS:
    Q_SLOT void setPersonImage(const QString &imagePath, const QString &personName);
    Q_SLOT void clearPersonImage();
    Q_SLOT void slotDisplayRecognizedPerson(const QString &name, const int &personId, 
                                             const QString &uuid, const QString &idcard);
    Q_SLOT void slotScreenTimeout();
    Q_SLOT void slotScreenWakeUp();
    Q_SLOT void slotUpdateTimerClock(); 
private://显示人脸主界面
    Q_SLOT void slotShowFaceHomeFrm(const int index);
    Q_SLOT void handleImageLoaded(const QPixmap &pixmap, const QString &imagePath);
private:
    Q_SLOT void onReady();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
	int mstatus = 0; //0:没进入设置界面是, 1:进入设置界面
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void createInstantPopupWithPlaceholder(const QString &name, int personId, const QString &idcard);
    void loadFaceImageAsync(const QString &name, int personId, const QString &uuid, const QString &idcard = QString());
    QString findFaceImageFast(int personId, const QString &name);
    void updatePopupWithImage(const QPixmap &pixmap, const QString &imagePath);
    bool isValidFaceCrop(const QImage &img);
    QString getEmployeeIdFromPersonId(int personId);
    QString getImagePathFromDatabase(const QString &employeeId);
    QString findStoredFaceImage(int personId, const QString &name, const QString &uuid, const QString &idcard);
public:
    CameraManager *m_pFaceCameraManager;  
    QString mPath;
    bool mDraw;      
private:
    QScopedPointer<FaceMainFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(FaceMainFrm)
    Q_DISABLE_COPY(FaceMainFrm)

};

#endif // FACEMAINFRM_H