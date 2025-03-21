#ifndef CONFIGACCESSRECORDSFRM_H
#define CONFIGACCESSRECORDSFRM_H

#include <QDialog>

//配置通行记录
class ConfigAccessRecordsFrmPrivate;
class ConfigAccessRecordsFrm : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigAccessRecordsFrm(QWidget *parent = nullptr);
    ~ConfigAccessRecordsFrm();
public://设置参数例表（保存全景图，保存人脸，保存陌生人记录）
    void setInitConfig(const bool, const bool, const bool);
    //获取全景
    bool getPanoramaState()const;
    //获取人脸
    bool getFaceState()const;
    //获取陌生人
    bool getStrangerState()const;
private:
    QScopedPointer<ConfigAccessRecordsFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture 
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif
private:
    Q_DECLARE_PRIVATE(ConfigAccessRecordsFrm)
    Q_DISABLE_COPY(ConfigAccessRecordsFrm)
};

#endif // CONFIGACCESSRECORDSFRM_H
