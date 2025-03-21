#ifndef StorageCapacityFrm_H
#define StorageCapacityFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//存储容量
class StorageCapacityFrmPrivate;
class StorageCapacityFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
	qint64 getCountTotal()
	{
		return mCountTotal;
	}
	void setCountTotal(qint64 c)
	{
		mCountTotal = c;
	}
	
    explicit StorageCapacityFrm(QWidget *parent = nullptr);
    ~StorageCapacityFrm();
	static inline StorageCapacityFrm *GetInstance(){static StorageCapacityFrm g;return &g;}	
private:
    virtual void setEnter();//进入
private:
	qint64 mCountTotal;
    QScopedPointer<StorageCapacityFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture 	
    void mouseDoubleClickEvent(QMouseEvent*);   
#endif 	 	
private:
    Q_DECLARE_PRIVATE(StorageCapacityFrm)
    Q_DISABLE_COPY(StorageCapacityFrm)
};

#endif // StorageCapacityFrm_H
