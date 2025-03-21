#pragma once
#include "interface/bface_types.h"

namespace bface {
/**
 * @brief SDK initializations and authentication, must call it at the very beginning before calling any
 *        other SDK APIs
 * @param[in] path searching path for models, priorboxes, license etc
 * @param[in] init_hw flag reserved, set true or false matters nothing.
 * @return BFACE_SUCCESS on SDK intialization success, otherwise an error code
 * @note this function is not thread-safe, you should call this thread only once before you use
 * the other api.
 */
BFACE_API
BFACE_STATUS bd_sdk_init(const char* path, bool init_hw);

/**
 * @brief baidu RK-FaceID-SDK initializations, must call it after bd_sdk_init and before calling any
 *        other FaceID SDK APIs
 * @return BFACE_SUCCESS on SDK intialization success, otherwise an error code
 * @note this function is not thread-safe, you should call this thread only once before you use
 * the other api.
 */
BFACE_API
BFACE_STATUS bface_init(void);

/**
 * @brief get the version of the current bface sdk
 * @param ver the output version buffer, at least 32 bytes
 * @param len the length of the input buffer.
 * @return  API status code, BFACE_SUCCESS
 */
BFACE_API
BFACE_STATUS bface_get_version(char* ver, int len);

/**
 * @brief detect faces of an image frame and output the bounding box of each face
 * @param[in] img  input image frame, currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[out] bbox_ptr  an array of detected faces bounding box information, see structure BoundingBox_t
 * @return BFACE_SUCCESS on success, otherwise an error code
 */
BFACE_API
BFACE_STATUS bface_detect_face(const Image_t& img, std::vector<BoundingBox_t>* bbox_ptr);

/**
 * @brief detect faces in one image and track the faces based on the previous tracking status,
 *        try to track all the given input bounding boxes, update the corresponding tracker's
 *        status and output the updated tracking id and tracking status for each bounding box.
 * @param[in] img  input image frame, currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[out] tracked_faces_ptr the output face bounding boxes, with tracking id and tracking
 *             status
 * @return BFACE_SUCCESS on success, otherwise an error code
 */
BFACE_API
BFACE_STATUS bface_detect_and_track(const Image_t& img, std::vector<TrackedBox_t>* tracked_faces_ptr);

/**
 * @brief detect landmark in one image.
 * @param[in] img  input face image, currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[in] bbox the input face bbox
 * @param[in] rectify_bbox whether or not rectify bbox about the forehead, true indicates do rectification to exlucde
 *            the forehead area or extend the bbox, false indicates no rectification, default true
 * @param[out] landmarks_ptr the output face landmark pointer
 * @return BFACE_SUCCESS on success, otherwise an error code
 * @note if rectify_bbox is true, the time of alignment would be longer than false because the function actually does
 *       alignment for twice, but the landmarks precision would be improved obviously on the other hand
 */
BFACE_API
BFACE_STATUS bface_alignment(const Image_t& img, BoundingBox_t& bbox, Landmark_t* landmarks_ptr, bool rectify_bbox = true);

/**
 * @brief detect all faces landmark in one image.
 * @param[in] img  input face image, currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[in] bbox the input face bbox array, user must ensures that each bbox's confidence is correct
 * @param[in] rectify_bbox whether or not rectify bbox about the forehead, true indicates do rectification to exlucde
 *            the forehead area or extend the bbox, false indicates no rectification, default true
 * @param[out] landmarks_ptr the output face landmark pointer array that matches the bbox one by one
 * @return BFACE_SUCCESS on success, otherwise an error code
 * @note if rectify_bbox is true, the time of alignment would be longer than false because the function actually does
 *       alignment for twice, but the landmarks precision would be improved obviously on the other hand
 */
BFACE_API
BFACE_STATUS bface_alignment_multi_face(const Image_t& img, std::vector<BoundingBox_t>& bbox, std::vector<Landmark_t>* landmarks_ptr, bool rectify_bbox = true);

/**
 * @brief bface_face_illumination takes an input detected face, and outputs a face illumination value.
 * @param[in] img input detected face.
 * @param[in] landmarks input face landmark.
 * @param[out] value_ptr output illumination[gray] value [0, 255], face is brighter when value is more closed to 255
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_face_illumination(const Image_t& img, const Landmark_t& landmarks, int* value_ptr);

/**
 * @brief bface_face_pose takes an input detected landmarks, and outputs a face pose value.
 * @param[in] landmarks input face landmark.
 * @param[out] pose_ptr output pose with pitch yaw roll, range in [-90, 90]
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_face_pose(const Landmark_t& landmarks, Pose_t* pose_ptr);

/**
 * @brief check whether an input face is an alive face.
 * @param[in] img input face image
 *            Currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[in] landmarks input face landmark.
 * @param[out] conf_ptr confidence of good face, normalized to 1
 * @return BFACE_SUCCESS on success, otherwise an error code
 * @todo this function doesn't do warp affine currently, needs to be implemented in the future
 */
BFACE_API
BFACE_STATUS bface_liveness(const Image_t& img, const Landmark_t& landmarks, float* conf_ptr);

/**
 * @brief check whether input faces in a image are alive or not.
 * @param[in] img input face image
 *            Currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[in] landmarks input face landmark array.
 * @param[out] conf_ptr confidence array of good face, normalized to 1
 * @return BFACE_SUCCESS on success, otherwise an error code
 */
BFACE_API
BFACE_STATUS bface_liveness_multi_face(const Image_t& img, const std::vector<Landmark_t>& landmarks, std::vector<float>* conf_ptr);

/**
 * @deprecated The data type of extracted feature is actually binary, the literal type of vector may be misleading.
 * @brief Takes an input detected face, and outputs a 512-byte binary feature.
 * @param[in] img input detected face
 * @param[in] landmarks input face landmark.
 * @param[out] feature_ptr output feature.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_DEPRECATED("The output feature variable type 'std::vector<float>*' may cause misunderstandings about feature data type, please use 'std::vector<Byte_t>*' to relpace it!")
BFACE_STATUS bface_extract_feature(const Image_t& img, const Landmark_t& landmarks, std::vector<float>* feature_ptr);

/**
 * @brief Takes an input detected face, and outputs a 512-byte binary feature.
 * @param[in] img input detected face
 * @param[in] landmarks input face landmark.
 * @param[out] feature_ptr output feature.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_extract_feature(const Image_t& img, const Landmark_t& landmarks, std::vector<Byte_t>* feature_ptr);

/**
 * @brief detect face occlusion status in one image.
 * @param[in] img  input face image, currently only support BFACE_PLANAR_U8C3YVU420SP.
 *            Both width and height must be even. For BFACE_PLANAR_U8C3_BGR image, the resolution should be smaller than 2688*2688
 * @param[in] bbox the bounding box of a face
 * @param[out] occ_score the 7 area occlusion score in a face ranges in [0.0, 1.0], the smaller the score is,
 *             the little occlusion exists
 * @note      The score sequence is: |left eye|right eye|nose|mouth|chin|left cheek|right cheek|
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_occlusion(const Image_t& img, const BoundingBox_t& bbox, std::vector<float>* occ_score);

/**
 * @deprecated This API is deprecated and it will be remove in future, Please use bface_mouthmask_multi_class to instead it!
 * @brief detect face moutmask status in one image.
 * @param[in] img  input face image, currently only support BFACE_PLANAR_U8C3YVU420SP.
 *            Both width and height must be even. For BFACE_PLANAR_U8C3_BGR image, the resolution should be smaller than 2688*2688
 * @param[in] bbox the bounding box of a face
 * @param[out] mask_score mouthmask score in a face ranges in [0.0, 1.0], the higher the score is,
 *             the more possible mouthmask exist
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_mouthmask(const Image_t& img, const BoundingBox_t& bbox, float* mask_score);

/**
* @brief detect face moutmask status in one image. support mouthmask model >= v6.0
* @param[in] img  input face image, currently only support BFACE_PLANAR_U8C3YVU420SP.
*            Both width and height must be even. For BFACE_PLANAR_U8C3_BGR image, the resolution should be smaller than 2688*2688
* @param[in] landmarks the landmarks of a face
* @param[out] mask_score output three kinds of score, no occlusion, mouthmask occlusion, not mouthmask occlusion, the score ranges in [0.0, 1.0], the highest the score is
*             the more possible class
* @return API status code, BFACE_SUCCESS indicates no error occurred.
*/
BFACE_API
BFACE_STATUS bface_mouthmask_multi_class(const Image_t& img, const Landmark_t& landmarks, std::vector<float> *mask_score);

/**
 * @brief bface_blur takes an input detected face, and outputs a face blur score.
 * @param[in] img input detected face.
 * @param[in] bbox the bounding box of a face.
 * @param[out] score_ptr output blur score pointer, score range is [0, 1], face blur will be more when score is closed to 1.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_blur(const Image_t& img, const BoundingBox_t &bbox, float* score_ptr);

/**
 * @brief bface_quality_score takes an input detected face, and outputs a face quality score for recognition.
 * @param[in] img input detected face.
 * @param[in] the bounding box of a face.
 * @param[out] score_ptr output quality score pointer, score range is [0, 1], face quality will be better when score is closed to 1.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_quality_score(const Image_t& img, const BoundingBox_t& bbox, float* score_ptr);

/**
 * @brief enhanced dark image frame and output the image frame enhanced
 * @param[in] src_img  input image frame, currently only support BFACE_INTERLEAVE_U8C3_BGR.
 *            Both width and height must be even. For BFACE_INTERLEAVE_U8C3_BGR image, the resolution should be smaller
 *            than 1920*1080.
 * @param[out] dst_img  image which has been retinex
 * @return BFACE_SUCCESS on success, otherwise an error code
 */
BFACE_API
BFACE_STATUS bface_retinex_image(const Image_t& src_img, Image_t *dst_img);

/**
 * @brief baidu SDK set authentication chip i2c 
 * @param[in] bus ranges from [0, BDCA_MAX_I2C)
 * @return BFACE_SUCCESS on setting SDK i2c success, otherwise an error code
 * @note input is the specifiction data within a certain range
 */
BFACE_API
BFACE_STATUS bd_sdk_set_i2c(uint8_t bus);

/**
 * @brief get the sn of crypto authentication device
 * @param sn the sn that is 9 bytes long
 * @param len the valid length of sn
 * @return  API status code, BFACE_SUCCESS
 */
BFACE_API
BFACE_STATUS bface_get_crypto_auth_dev_sn(uint8_t sn[20], int* len);

// =====================================================================================================
/**
 * @brief The functions of face detection and alignment + quality detection + living detection are
 *        encapsulated into an interface. The user first calls the configuration interface to set
 *        the incoming parameters and callback function, and then calls the filling interface to
 *        continuously fill the image data of the input video. Finally, the callback function
 *        is used to process the returned results to complete the living judgment of the image.
 * @param[in] param input pipeline config params.
 * @param[in] callback user callback function.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_liveness_pipeline_set(LivenessPipelineCallback_t callback = nullptr, 
        const LivenessPipelineParam_t &param = LivenessPipelineParam_t());

/**
 * @brief The functions of face detection and alignment + quality detection + living detection are
 *        encapsulated into an interface. The user first calls the configuration interface to set
 *        the incoming parameters and callback function, and then calls the filling interface to
 *        continuously fill the image data of the input video. Finally, the callback function
 *        is used to process the returned results to complete the living judgment of the image.
 * @param[in] rgb_img input image, now only support rgb and rgb mask
 * @param[in] nir_img input image, now only support rgb.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_liveness_pipeline_fill(const SmartImage_t &rgb_img, const SmartImage_t &nir_img);

/**
 * @brief Face detection and alignment + quality + feature extraction function encapsulation to an interface,
 *        the user first call the configuration interface to set the incoming parameter and a callback function,
 *        and then call filling interface, fill in the input video image data, and finally through the callback
 *        function to handle human face feature extraction result, may through the formulation of the eigenvalue
 *        extraction model complete face feature extraction.
 * @param[in] param input pipeline config params.
 * @param[in] callback user callback function.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_extract_pipeline_set(FeatureExtractPipelineCallback_t callback = nullptr,
        const FeatureExtractPipelineParam_t &param = FeatureExtractPipelineParam_t());

/**
 * @brief Face detection and alignment + quality + feature extraction function encapsulation to an interface,
 *        the user first call the configuration interface to set the incoming parameter and a callback function,
 *        and then call filling interface, fill in the input video image data, and finally through the callback
 *        function to handle human face feature extraction result, may through the formulation of the eigenvalue
 *        extraction model complete face feature extraction.
 * @param[in] rgb_img input image, now only support rgb.
 * @param[in] nir_img input image, now only support nir.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_extract_pipeline_fill(const SmartImage_t &rgb_img, const SmartImage_t &nir_img);

/**
 * @brief Based on human face feature extraction interface "complete face 1:1 ratio,
 *        the user first call the configuration interface to set the incoming parameter and
 *        a callback function, and then call filling interface, fill in the input video image data,
 *        and finally through the callback function returns the result, complete the comparison of
 *        two face image similarity, return the result of two face image similarity and results
 * @param[in] img input ID image, now only support rgb.
 * @param[in] param input pipeline config params.
 * @param[in] callback user callback function.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_1v1_pipeline_set(const SmartImage_t &img = nullptr, 
        Feature1V1PipelineCallback_t callback = nullptr,
        const Feature1V1PipelineParam_t &param = Feature1V1PipelineParam_t());

/**
 * @brief Based on human face feature extraction interface "complete face 1:1 ratio,
 *        the user first call the configuration interface to set the incoming parameter and
 *        a callback function, and then call filling interface, fill in the input video image data,
 *        and finally through the callback function returns the result, complete the comparison of
 *        two face image similarity, return the result of two face image similarity and results
 * @param[in] rgb_img input image, now only support rgb.
 * @param[in] nir_img input image, now only support nir.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_1v1_pipeline_fill(const SmartImage_t &rgb_img, const SmartImage_t &nir_img);

/**
 * @brief Based on human face feature extraction interface "complete face 1: N retrieval,
 *        the user first call the configuration interface to set the incoming parameter
 *        and a callback function, and then call filling interface, fill in the input video image data,
 *        and finally through the callback function returns results, to complete the library on
 *        the floor of the input image face feature in retrieval, returns the retrieval results and similarity;
 * @param[in] callback user callback function.
 * @param[in] param input pipeline config params.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_1vn_pipeline_set(Feature1VnPipelineCallback_t callback = nullptr,
        const Feature1VnPipelineParam_t &param = Feature1VnPipelineParam_t());

/**
 * @brief Based on human face feature extraction interface "complete face 1: N retrieval,
 *        the user first call the configuration interface to set the incoming parameter
 *        and a callback function, and then call filling interface, fill in the input video image data,
 *        and finally through the callback function returns results, to complete the library on
 *        the floor of the input image face feature in retrieval, returns the retrieval results and similarity;
 * @param[in] rgb_img input image, now only support rgb.
 * @param[in] nir_img input image, now only support nir.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_feature_1vn_pipeline_fill(const SmartImage_t &rgb_img, const SmartImage_t &nir_img);

/** ------------------------- Dividing line -----------------------------------**/
/** ---All the APIs below are invalid now, Do not call them at this version--- **/

/**
 * @brief baidu sdk  uninitializations, must call it at the last after exit.
 * @return BFACE_SUCCESS on SDK intialization success, otherwise an error code
 * @note this function is not thread-safe, you should call this thread only once before you use
 * the other api.
 */
BFACE_API
BFACE_STATUS bd_sdk_uninit(void);

} /// namespace bface