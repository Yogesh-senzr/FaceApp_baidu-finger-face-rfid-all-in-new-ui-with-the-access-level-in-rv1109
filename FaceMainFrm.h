#ifndef FACEMAINFRM_H
#define FACEMAINFRM_H
#include "RKCamera/Camera/cameramanager.h"
#include <QWidget>

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
    //这个函数有问题，会导致 FaceMainFrm 这个类重复实例化，在最开始的时候已经实例化过该类的一个实例了
    //static inline FaceMainFrm *GetInstance(){static FaceMainFrm g;return &g;}	
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
private://显示人脸主界面
    Q_SLOT void slotShowFaceHomeFrm(const int index);
private:
    Q_SLOT void onReady();
private:
	int mstatus = 0; //0:没进入设置界面是, 1:进入设置界面
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
        
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
