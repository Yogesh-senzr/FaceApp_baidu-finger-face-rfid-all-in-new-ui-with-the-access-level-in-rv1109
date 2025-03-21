#ifndef ADDUSERFRM_H
#define ADDUSERFRM_H

#include <QWidget>

//新增用户
class AddUserFrmPrivate;
class AddUserFrm : public QWidget
{
    Q_OBJECT
public:
    explicit AddUserFrm(QWidget *parent = nullptr);
    ~AddUserFrm();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
public://显示人脸主界面
    Q_SIGNAL void sigShowFaceHomeFrm(const int index = 1);
    Q_SIGNAL void sigShowFaceHomeFrmAction(const int index = 1);//1:Add,2:Modify
    Q_SIGNAL void sigModifyUser();
    Q_SIGNAL void sigModifyUserRefreshGrid();
public:
    void modifyRecord(QString aName,QString aIDCard,QString aCardNo,QString asex,QString apersonid,QString auuid);
    static inline AddUserFrm *GetInstance(){static AddUserFrm g;return &g;}        
private:
    int mModify=0;
    QString mPath;
    bool mDraw;       
    Q_SLOT void slotCaptureFaceButton();
    Q_SLOT void slotRegFaceButton();
private://标题返回按钮
    Q_SLOT void slotReturnSuperiorClicked();
    Q_SLOT void slotTextChanged(const QString &);
public:
    Q_SLOT void slotPersonInfo(const QString &,const QString &,const QString &,const QString &); 
    Q_SLOT void slotModifyUser();    
private:
    void hideEvent(QHideEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<AddUserFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture    
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif     
private:
    Q_DECLARE_PRIVATE(AddUserFrm)
    Q_DISABLE_COPY(AddUserFrm)
};

#endif // ADDUSERFRM_H
