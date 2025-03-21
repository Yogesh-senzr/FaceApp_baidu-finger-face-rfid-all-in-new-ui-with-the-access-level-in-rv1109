#ifndef CBAIDUFACEENGINE_H
#define CBAIDUFACEENGINE_H


#include "baidu_face_sdk.h"

#include "rga/rga.h"
#include "rga/RgaApi.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "helper/face_utils.hpp"
#include "helper/image_convert.hpp"
#include "helper/io/io.h"
#include "helper/timer/timer.h"
#include "interface/faceid_interface.h"
#include "interface/feature_interface.h"
#include "interface/bface_types.h"

#include "SharedInclude/GlobalDef.h"

#include <iostream>
#include <string>

class CBaiduFaceEngine
{
public:
    CBaiduFaceEngine();
    virtual ~CBaiduFaceEngine();
    void uninit();
    int activateOnline(std::string appid, std::string sdkKey, std::string activeKey);
    int activateOffline(std::string appid, std::string sdkKey, std::string activeKey);

    virtual int init(DetectMode detectMode, int nScale, int nMask);

    virtual int faceDetect(bface::Image_t pRGBImg,  LPBAIDU_MultiFaceInfo pFaceInfo, int* pNum);
  
    virtual int livenessDetect(LPASVLOFFSCREEN pOff, LPBAIDU_MultiFaceInfo pFaceInfo, int* pLiveness);
    int faceFeatureCompare(LPBAIDU_FaceFeature faceFeature1, LPBAIDU_FaceFeature faceFeature2, double* similar);
    
    int faceFeatureExtract(bface::Image_t pRGBImg,std::vector<bface::BoundingBox_t> bbox_list, BAIDU_SingleFaceInfo faceInfo, LPBAIDU_FaceFeature pFaceFeature, BAIDU_RegisterOrNot mode =
            BAIDU_RECOGNITION, int mask = 0);

    virtual int setLivenessThreshold(double rgbThreshold, double irThreshold);

    virtual int faceQuality(bface::Image_t pRGBImg, std::vector<bface::Byte_t>* feature, LPBAIDU_MultiFaceInfo pFaceInfo, double *threshold);
    
    int init();

    int getLivenessType(){return livenessType;}
private:
    void* mHandle;
    int livenessType;
    Face_S *mFaceFake;
    CORE_FACE_S *getCoreFaceFake(unsigned int nTrackID, unsigned int nExistTrackID[MAX_FACES]);
};

#endif // CBAIDUFACEENGINE_H
