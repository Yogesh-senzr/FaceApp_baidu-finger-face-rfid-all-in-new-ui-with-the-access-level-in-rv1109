#include "CBaiduFaceEngine.h"


#include "MessageHandler/Log.h"

#include "interface/bface_types.h"
#include "helper/face_utils.hpp"
#include "helper/image_convert.hpp"
#include "helper/timer/timer.h"
#include "interface/faceid_interface.h"
#include "interface/bd_default_param.h"
#include "baidu_face_sdk.h"


#include "Config/ReadConfig.h"

#include "gflags/gflags.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define MAX_FACES (10)
const int LIVENESS_RGB = 1;
const int LIVENESS_IR = 2;
const int LIVENESS_IMAGEQUALITY = 3;

#define MERR_INVALID_PARAM -1
#define MERR_BAD_STATE -2

CBaiduFaceEngine::CBaiduFaceEngine() :
mHandle(nullptr)
{
	mFaceFake = (Face_S*) malloc(sizeof(Face_S));
	memset(mFaceFake, 0, sizeof(Face_S));
	mFaceFake->nTrackID = -1;
}

CBaiduFaceEngine::~CBaiduFaceEngine()
{
    //uninit();
}

int CBaiduFaceEngine::activateOnline(std::string appid, std::string sdkKey, std::string activeKey)
{
    if (appid.size() == 0 || sdkKey.size() == 0)
    {
        //return MERR_INVALID_PARAM;
    }

   return -1;
}

int CBaiduFaceEngine::activateOffline(std::string appid, std::string sdkKey, std::string activeKey)
{

    int ret = MERR_BAD_STATE;
    if (appid.size() == 0 || sdkKey.size() == 0 || activeKey.size() == 0)
    {
        //return MERR_INVALID_PARAM;
    }

    ret = MERR_BAD_STATE;
    if (!access("/param/license.txt", F_OK))
    {
        //ret = BAIDUOfflineActivation("/param/license.txt"); //liwen
    }
    return ret;
}

int CBaiduFaceEngine::init(DetectMode detectMode, int nScale, int nMask)
{
    //long res = BaiduInitEngine(detectMode, BAIDU_OP_0_ONLY, nScale, 5, nMask, &mHandle);
    //参考 ASFInitEngine, 已经 初始过了
    long res = MOK; // -1;
    if (res == MOK)
    {
        if ((nMask & BAIDU_LIVENESS) == BAIDU_LIVENESS)
        {
            livenessType = LIVENESS_RGB;
        } else if ((nMask & BAIDU_IR_LIVENESS) == BAIDU_IR_LIVENESS)
        {
            livenessType = LIVENESS_IR;
        }else if((nMask & BAIDU_IMAGEQUALITY) == BAIDU_IMAGEQUALITY)
        {
            livenessType = LIVENESS_IMAGEQUALITY;
        }
    }
    return res;
}

int CBaiduFaceEngine::faceDetect(bface::Image_t pRGBImg,  LPBAIDU_MultiFaceInfo pFaceInfo, int* pNum)
{    
    BAIDU_MultiFaceInfo faceInfo = { nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr };
#ifdef OUTPUT_LOG
    long long start = Utils::getCurrentTimeMsec();
#endif
    long res = -1;


{
    unsigned int m_nExistTrackID[MAX_FACES] = { 0 };
	int nMatchCoreFaceIndex = 0; //当前脸最大那个人的索引
	int nFaceNum = 0;
	int nCurTrackID = 0;
	CORE_FACE_S *pstUpdateFaceInfo[MAX_FACES] = { 0 };
    static int m_nDismissFaceCount = 0;
    static CORE_FACE_S *m_stCurCoreFace = ISC_NULL;    

	std::vector<bface::TrackedBox_t> stTrackedFaceList;
   
		bface::BFACE_STATUS status = bface::bface_detect_and_track(pRGBImg, &stTrackedFaceList);        

		if (status == bface::BFACE_SUCCESS)
		{
			float nMaxFaceSize = 0;            
			for (int i = 0; i < stTrackedFaceList.size(); i++)
			{
				if ((i + 1) == MAX_FACES)
				{
					break;
				}                
      
				//如果质量低于 BD_TRACKING_RGB_CONF 就忽略此人脸
				if (stTrackedFaceList[i].tracking_status == bface::TRACKED && stTrackedFaceList[i].bbox.conf >= ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold())
				{
					float x = stTrackedFaceList[i].bbox.rect.left;
					float y = stTrackedFaceList[i].bbox.rect.top;
					float width = stTrackedFaceList[i].bbox.rect.width;
					float height = stTrackedFaceList[i].bbox.rect.height;
                    
					pstUpdateFaceInfo[i] = getCoreFaceFake(stTrackedFaceList[i].tracking_id, m_nExistTrackID);
					if (pstUpdateFaceInfo[i]->attr_info.liveness_ir == 1)
					{
						pstUpdateFaceInfo[i]->track_count++;
					}
					pstUpdateFaceInfo[i]->stFaceRect.nX = x;
					pstUpdateFaceInfo[i]->stFaceRect.nY = y;
					pstUpdateFaceInfo[i]->stFaceRect.nWidth = width;
					pstUpdateFaceInfo[i]->stFaceRect.nHeight = height;
					if (pstUpdateFaceInfo[i]->enFaceType != CORE_FACE_RECT_TYPE_MATCH)
					{
						pstUpdateFaceInfo[i]->enFaceType = CORE_FACE_RECT_TYPE_MOVING;
					}

					m_nExistTrackID[i] = stTrackedFaceList[i].tracking_id;

					if (width * height > nMaxFaceSize)
					{
						nMaxFaceSize = width * height;
						nMatchCoreFaceIndex = i;
						nCurTrackID = stTrackedFaceList[i].tracking_id;
					}

					nFaceNum++;
				}
			}
		}

        *pNum = nFaceNum;		
       
		if (nFaceNum < 1)
		{

			int nDismissWaitFrame = 6;
			m_nDismissFaceCount++;
			m_nDismissFaceCount = (m_nDismissFaceCount > (10 * nDismissWaitFrame)) ? (nDismissWaitFrame + 1) : (m_nDismissFaceCount);
			if (m_nDismissFaceCount == nDismissWaitFrame) //多少帧内没人就发dismiss消息
			{

			}
			return res;

		}
   	
	}    
  
    res = MOK;
    return res;
}

int CBaiduFaceEngine::faceFeatureExtract(bface::Image_t pRGBImg,std::vector<bface::BoundingBox_t> bbox_list, BAIDU_SingleFaceInfo faceInfo, LPBAIDU_FaceFeature pFaceFeature,
                                    BAIDU_RegisterOrNot mode, int mask)
{
#ifdef OUTPUT_LOG
    long long start = Utils::getCurrentTimeMsec();
#endif
    //MRESULT res = ASFFaceFeatureExtractEx(mHandle, pOff, &faceInfo, pFaceFeature, mode, mask);

    /// 提取人脸关键点
    bface::Landmark_t landmarks;
    //std::vector<bface::BoundingBox_t> bbox_list;    
    bface::BFACE_STATUS status = bface_alignment(pRGBImg, bbox_list[0], &landmarks);
    
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_alignment failed error : " << status << std::endl;
        return -1;
    }
    
    /// 提取人脸特征
    bface::helper::MicrosecondTimer timer;
    std::vector<bface::Byte_t> feature;
    status = bface_extract_feature(pRGBImg, landmarks, &feature);
    int timing = timer.end();
    //std::cout << "bface_extract_feature cost : "<< timing << " us." << std::endl;
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_extract_feature failed error : " << status << std::endl;
        return -1;
    }
    
    /// 附注：以下步骤仅用于演示特征数据的文件读写操作，实际应用时可忽略
    /// 将特征保存至文件
    #if 0
    std::string file_name = "feature.bin";
    write_feature_to_file(feature, file_name);
    /// 从文件中读取特征
    std::vector<Byte_t> feat;
    read_feature_from_file(file_name, feat);
    /// 校验一致性
    for (size_t i = 0; i < feature.size(); i++) {
        if (feat[i] != feature[i]) {
            std::cerr << "feature IO is error : " << std::endl;
            break;
        }
        if (i < 5) {
            std::cout << "feature data [" << i << "] : " << int(feat[i]) << std::endl;
        }
    }
    #endif  


#ifdef OUTPUT_LOG
    char temp[256];
    sprintf(temp,"fe time: %lld res: %d\n",Utils::getCurrentTimeMsec() - start, res);
    qDebug()<<temp;
#ifdef DUMP_DATA
    Utils::saveLog(temp);
#endif
#endif
    return 0;
}
#if 0
int CBaiduFaceEngine::faceFeatureCompare(LPASF_FaceFeature faceFeature1, LPASF_FaceFeature faceFeature2, double* similar)
{
    MFloat level = 0;
    MRESULT res = ASFFaceFeatureCompare(mHandle, faceFeature1, faceFeature2, &level);
    if (res == MOK)
    {
        *similar = static_cast<double>(level);
    }
    return res;
}
#endif 
int CBaiduFaceEngine::livenessDetect(LPASVLOFFSCREEN pOff, LPBAIDU_MultiFaceInfo pFaceInfo, int* pLiveness)
{
    int res = -1;//MERR_UNKNOWN;
#ifdef OUTPUT_LOG
    long long start = Utils::getCurrentTimeMsec();
#endif
    if (livenessType == LIVENESS_RGB)
    {
        //res = ASFProcessEx(mHandle, pOff, pFaceInfo, BAIDU_LIVENESS);
    } else if (livenessType == LIVENESS_IR)
    {
        //res = ASFProcessEx_IR(mHandle, pOff, pFaceInfo, BAIDU_IR_LIVENESS);
    }
    if (res == MOK)
    {
#if 0        
        ASF_LivenessInfo info = { nullptr, 0 };
        if (livenessType == LIVENESS_RGB)
        {
            res = ASFGetLivenessScore(mHandle, &info);
        } else
        {
            res = ASFGetLivenessScore_IR(mHandle, &info);
        }
        if (res == MOK && info.num)
        {
            *pLiveness = info.isLive[0];
        }
#endif         
    }
#ifdef OUTPUT_LOG
    char temp[256];
    sprintf(temp,"fl time: %lld res: %d live: %d\n",Utils::getCurrentTimeMsec() - start, res, *pLiveness);
    qDebug()<<temp;
#ifdef DUMP_DATA
    Utils::saveLog(temp);
#endif
#endif
    return res;
}
#if 0
int CBaiduFaceEngine::maskDetecct(LPASVLOFFSCREEN pOff, LPASF_MultiFaceInfo pFaceInfo, int* pMask)
{
#ifdef OUTPUT_LOG
    long long start = Utils::getCurrentTimeMsec();
#endif
    MRESULT res = ASFProcessEx(mHandle, pOff, pFaceInfo, ASF_MASKDETECT);
    if (res == MOK)
    {
        ASF_MaskInfo info = { nullptr, 0 };
        res = ASFGetMask(mHandle, &info);
        if (res == MOK && info.num)
        {
            *pMask = info.maskArray[0];
        }
    }
#ifdef OUTPUT_LOG
    char temp[256];
    sprintf(temp,"mask time: %lld res: %d mask: %d\n",Utils::getCurrentTimeMsec() - start, res, *pMask);
    qDebug()<<temp;
#ifdef DUMP_DATA
    Utils::saveLog(temp);
#endif
#endif
    return res;
}
#endif 
int CBaiduFaceEngine::setLivenessThreshold(double rgbThreshold, double irThreshold)
{    
    BAIDU_LivenessThreshold threshold = { 0, 0 };
    threshold.thresholdmodel_IR = irThreshold;
    threshold.thresholdmodel_BGR = rgbThreshold;
   
   return 0;
}


//包含在 FaceFeatureExtract 中
int CBaiduFaceEngine::faceQuality(bface::Image_t pRGBImg,std::vector<bface::Byte_t>* feature, LPBAIDU_MultiFaceInfo pFaceInfo, double *threshold)
{
#ifdef OUTPUT_LOG
    long long start = Utils::getCurrentTimeMsec();
#endif
    BAIDU_ImageQualityInfo info = { nullptr, 0 };

    /// 执行人脸检测
    std::vector<bface::BoundingBox_t> bbox_list;
    bface::BFACE_STATUS status = bface_detect_face(pRGBImg, &bbox_list);
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_detect_face failed error : " << status << std::endl;
        return -1;
    }
    if (bbox_list.size() == 0) {
        //std::cout << "No faces detected from input picture : " << FLAGS_image_file << std::endl;
        return 0;
    }
    /// 提取人脸关键点
    bface::Landmark_t landmarks;
    status = bface_alignment(pRGBImg, bbox_list[0], &landmarks);
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_alignment failed error : " << status << std::endl;
        return -1;
    }
    int cost_time = 0;
    bface::Pose_t face_pose;
    int luminance_value = 0;
    /// 计算人脸得分
    float quality_score = 0.f;
    bface::helper::MicrosecondTimer quality_timer;

    bface::BoundingBox_t mybox;

    //status = bface_quality_score(pRGBImg, bbox_list[0], &quality_score);
    status = bface_quality_score(pRGBImg, mybox, &quality_score);
    cost_time = quality_timer.end();
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_quality_score failed error : " << status << std::endl;
        return -1;
    }
    std::cout << "face quality_score cost " << cost_time << " us." << std::endl;
    std::cout << "<--- quality score ---> " << '\n';
    std::cout << '\t' << " quality score value : " << quality_score << std::endl;

    /// 计算人脸模糊度
    float blur_score = 0;
    bface::helper::MicrosecondTimer blur_timer;
    status = bface_blur(pRGBImg, bbox_list[0], &blur_score);
    cost_time = blur_timer.end();
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_blur failed error : " << status << std::endl;
        return -1;
    }
    std::cout << "bface_blur cost " << cost_time << " us." << std::endl;
    std::cout << "<--- blur score ---> " << '\n';
    std::cout << '\t' << " blur score value : " << blur_score << std::endl;

    /// 计算头部姿态
    bface::helper::MicrosecondTimer pose_timer;
    status = bface_face_pose(landmarks, &face_pose);
    cost_time = pose_timer.end();
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_face_pose failed error : " << status << std::endl;
        return -1;
    }
    std::cout << "bface_face_pose cost " << cost_time << " us." << std::endl;
    std::cout << "<--- pose score ---> " << '\n'
            << '\t' << " pose pitch: " << face_pose.pitch << '\n'
            << '\t' << " pose yaw: " << face_pose.yaw << '\n'
            << '\t' << " pose roll: " << face_pose.roll << std::endl;
    
    /// 计算人脸遮挡情况
    bface::helper::MicrosecondTimer occlusion_timer;
    std::vector<float> score;
    std::vector<float> occlusion_score;
    status = bface_occlusion(pRGBImg, bbox_list[0], &occlusion_score);
    cost_time = occlusion_timer.end();
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_occlusion failed error : " << status << std::endl;
        return -1;
    }
    std::cout << " bface_occlusion cost : " << cost_time << " us." << std::endl;
    std::cout << "<--- occlusion score ---> " << '\n' 
            << '\t' << " Left eye : " << occlusion_score[0] << '\n'
            << '\t' << " Right eye : " << occlusion_score[1] << '\n'
            << '\t' << " Nose : " << occlusion_score[2] << '\n'
            << '\t' << " Mouth : " << occlusion_score[3] << '\n'
            << '\t' << " Left cheek : " << occlusion_score[4] << '\n'
            << '\t' << " Right cheek : " << occlusion_score[5] << '\n'
            << '\t' << " Chin : " << occlusion_score[6] << std::endl;

    /// 计算人脸亮度
    bface::helper::MicrosecondTimer illumination_timer;
    status = bface_face_illumination(pRGBImg, landmarks, &luminance_value);
    cost_time = illumination_timer.end();
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_face_illumination failed error : " << status << std::endl;
        return -1;
    }
    std::cout << "bface_face_illumination cost " << cost_time << " us." << std::endl;
    std::cout << "<--- luminance score ---> " << '\n';
    std::cout << '\t' << " luminance value: " << luminance_value << std::endl;


    /// 提取人脸关键点
    //bface::Landmark_t landmarks;
    status = bface_alignment(pRGBImg, bbox_list[0], &landmarks);
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_alignment failed error : " << status << std::endl;
        return -1;
    }
    /// 提取人脸特征
    bface::helper::MicrosecondTimer timer;
    //std::vector<bface::Byte_t> feature;
    status = bface_extract_feature(pRGBImg, landmarks, feature);
    int timing = timer.end();
    //std::cout << "bface_extract_feature cost : "<< timing << " us." << std::endl;
    if (status != bface::BFACE_SUCCESS) {
        std::cerr << "bface_extract_feature failed error : " << status << std::endl;
        return -1;
    }    

    *threshold = quality_score;

#ifdef OUTPUT_LOG
    char temp[256];
    sprintf(temp,"fq time: %lld res: %d\n",Utils::getCurrentTimeMsec() - start, res);
    qDebug()<<temp;
#ifdef DUMP_DATA
    Utils::saveLog(temp);
#endif
#endif
    return status;
}
#if 0
int CBaiduFaceEngine::detectFace3DAngle(LPASVLOFFSCREEN pOff, LPASF_MultiFaceInfo pFaceInfo, LPASF_Face3DAngle p3DAngleInfo)
{
    if (p3DAngleInfo != NULL)
    {
        MRESULT res = ASFProcessEx(mHandle, pOff, pFaceInfo, ASF_FACE3DANGLE);
        if (res == MOK)
        {
            res = ASFGetFace3DAngle(mHandle, p3DAngleInfo);
        }
        return res;
    }
    return MERR_INVALID_PARAM;
}
#endif 
int CBaiduFaceEngine::init()
{

}

void CBaiduFaceEngine::uninit()
{
 
  //  bface::bd_sdk_uninit();

}


CORE_FACE_S * CBaiduFaceEngine::getCoreFaceFake(unsigned int nTrackID, unsigned int nExistTrackID[MAX_FACES])
{
    //pthread_mutex_t m_stCoreFacesLock;

	//pthread_mutex_lock(&m_stCoreFacesLock);

	unsigned long long lTrackID = ++nTrackID;
	CORE_FACE_S *pFoundCoreFace = ISC_NULL;
	//查找当前内存里面是否有对应 trackid 的人
	for (int i = 0; i < MAX_FACES; i++)
	{
		if (mFaceFake->stCoreFaces[i].track_id == lTrackID)
		{
			pFoundCoreFace = &(mFaceFake->stCoreFaces[i]);
			//pthread_mutex_unlock(&m_stCoreFacesLock);
			return pFoundCoreFace;
		}
	}

	//上面的操作找不到内存里面对应的人，则从内存里面从头找一个不在当前nExistTrackID 里面的人
	if (pFoundCoreFace == ISC_NULL)
	{
		pFoundCoreFace = &(mFaceFake->stCoreFaces[0]);
	}

	for (int i = 0; i < MAX_FACES; i++)
	{
		if (nExistTrackID != ISC_NULL)
		{
			//对比当前镜头前的人所对应的trackid，
			bool bEixst = false;
			for (int j = 0; j < MAX_FACES; j++)
			{
				if (mFaceFake->stCoreFaces[i].track_id == nExistTrackID[j] && nExistTrackID[j] != 0)
				{
					bEixst = true;
					break;
				}
			}
			if (bEixst == true)
			{
				continue;
			}
		}
		pFoundCoreFace = &(mFaceFake->stCoreFaces[i]);
		break;
	}

	if (pFoundCoreFace != ISC_NULL)
	{
		memset(pFoundCoreFace, 0, sizeof(CORE_FACE_S));
		pFoundCoreFace->track_id = lTrackID;
	}
	//pthread_mutex_unlock(&m_stCoreFacesLock);
 
	return pFoundCoreFace;

  return ISC_NULL;
}
