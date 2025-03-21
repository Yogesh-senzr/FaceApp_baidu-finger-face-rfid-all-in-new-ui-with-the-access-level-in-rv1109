#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QSettings>

#include "../Drm/display.h"
#include "SharedInclude/CallBindDef.h"
#include "SharedInclude/GlobalDef.h"

typedef FF_CALLBACK(void(int nPixelFormat, unsigned long  nYuvVirAddr0, unsigned long  nYuvPhyAddr0, int nWidth0, int nHeight0, int nSize0, int rotation0,
                         unsigned long  nYuvVirAddr1, unsigned long  nYuvPhyAddr1, int nWidth1, int nHeight1, int nSize1, int rotation1)) CameraPreviewYUVCallBack;
class CameraManagerPrivate;
class CameraManager : public QObject
{
    Q_OBJECT
public:
    explicit CameraManager(const QSize RgaResolution,const bool RgaMirrored,const qint32 RgaRotation,const QSize IrResolution,const bool IrMirrored,const qint32 IrRotation, const int w, const int h, QObject *parent= Q_NULLPTR);
    ~CameraManager();
public:
    void uninit();
    bool startPreview(bool bDualCamera, QSize size);
    void stopPreview();

    void startAiProcess();
    void stopAiProcess();

    bool startIrPreview();
    void stopIrPreview();

    void setFaceInfo(enum display_face_type *enType, int nSize);
    void setFaceRect(MRECT *stRect,int nSize);

    void setCameraPreviewYUVDataCall(int nPixelFormat, unsigned long  nYuvVirAddr0, unsigned long  nYuvPhyAddr0, int nWidth0, int nHeight0, int nSize0, int rotation0,
                                     unsigned long  nYuvVirAddr1, unsigned long  nYuvPhyAddr1, int nWidth1, int nHeight1, int nSize1, int rotation1);

    void setCameraPreviewYUVCallBack(CameraPreviewYUVCallBack call);

    bool takePhotos(char **ppDstRgbData,int *pDstRgbSize, char **ppDstIrData,int *pDstIrSize);
public:
    Q_SLOT void slotDisMissMessage();//当前无人脸
    Q_SLOT void slotDrawFaceRect(const QList<CORE_FACE_RECT_S>);//画人脸移动
private:
    QScopedPointer<CameraManagerPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CameraManager)
    Q_DISABLE_COPY(CameraManager)
};

#endif // CAMERAMANAGER_H
