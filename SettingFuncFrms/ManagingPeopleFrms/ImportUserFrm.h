#ifndef ImportUserFrm_H
#define ImportUserFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//导入用户
class ImportUserFrmPrivate;
class ImportUserFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit ImportUserFrm(QWidget *parent = nullptr);
    ~ImportUserFrm();
private:
    Q_SLOT void slotImportButtonClicked();
    //处理进度（是否完成、当前处理条数、成功记录、失败记录）
    Q_SLOT void slotProgressShell(const bool, const int total, const int, const int, const int);
private:
    Q_SIGNAL void sigParseXlsxPath(const QString);
private:
    virtual void setEnter();
private:
    QScopedPointer<ImportUserFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif    
private:
    Q_DECLARE_PRIVATE(ImportUserFrm)
    Q_DISABLE_COPY(ImportUserFrm)
};

#endif // ImportUserFrm_H
