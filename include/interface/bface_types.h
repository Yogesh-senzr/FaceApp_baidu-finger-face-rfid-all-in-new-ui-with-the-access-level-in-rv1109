#pragma once
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

#include "interface/bface_export.h"

/**
 * @brief the root namespace
 */
namespace bface {
/// @brief API status code
typedef enum {
    /// Successfully
    BFACE_SUCCESS = 0,
    /// Invalid license, no license file or license expired
    BFACE_INVALID_LICENSE = -1,
    /// Invalid model, no model found or loading model failed
    BFACE_INVALID_MODEL = -2,
    /// sdk is not initialized
    BFACE_UNINITIALIZED = -3,
    /// sdk runtime environment is not installed properly
    BFACE_INVALID_ENV = -4,
    /// invalid function parameters
    BFACE_INVALID_PARAM = -5,
    /// invalid pointer
    BFACE_INVALID_POINTER = -6,
    /// preprocess failed
    BFACE_PREOP_FAILED = -7,
    /// inference error
    BFACE_INFER_FAILED = -8,
    /// hardware initialization failure
    BFACE_INIT_HW_FAILED = -9,
    /// feature search failure
    BFACE_SEARCH_FAILED = -10,
    /// no config.toml found
    BFACE_NO_CONFIG_FOUND = -11,
    /// invalid config.toml parameters
    BFACE_INVALID_CONFIG = -12,
    /// post process failed.
    BFACE_POSTOP_FAILED = -13,
    /// crypto authentication failed
    BFACE_CA_FAILED = -14,
    /// not implemented
    BFACE_NOT_IMPLEMENTED = -15,
    /// current API is not valid due to invalid models or priorbox or unenough memory etc.
    BFACE_API_INVALID = -16,
    /// uninit hardware failed
    BFACE_UNINIT_HW_FAILED = -17,
    /// setting i2c failed
    BFACE_SET_I2C_FAILED = -18,
    /// invode platform interface error
    BFACE_PLATFORM_ERROR = -19,
    /// filter threshold error
    BFACE_FILTER_ERROR = -20,
    /// pipeline running, not has result
    BFACE_PIPELINE_RUNNING = -21
} BFACE_STATUS;

#if UINTPTR_MAX == 0xffffffff
typedef uint32_t bface_pointer_type;
#elif UINTPTR_MAX == 0xffffffffffffffff
typedef uint64_t bface_pointer_type;
#else
#error "non support hardware platform"
#endif

#define TO_BFACE_POINTER_TYPE(x) bface_pointer_type(x)
#define TO_STD_POINTER_TYPE(x, type) (type)(x)

/**
 * @brief describe the type of an image
 */
typedef enum {
    /// RGB image from RGB camera.
    BFACE_IMAGE_RGB = 0x00,
    /// NIR image from NIR camera.
    BFACE_IMAGE_NIR = 0x01,
    /// detph image from depth camera.
    BFACE_IMAGE_DEPTH = 0x02,
    /// RGB ID photo.
    BFACE_IMAGE_RGB_ID= 0x06
} IMAGE_TYPE;

typedef enum {
    /// RGB live photo feature
    RGB_FEATURE = 0,
    /// NIR live photo feature
    NIR_FEATURE = 1,
    /// RGB ID photo feature
    RGB_ID_FEATURE = 5,
    STORE_TYPE_ALL
} StoreType;

/**
 * @brief describe the pixel format of an image,
 */
typedef enum {
    /// 1 channels, unsigned, 8 bits depth, planar format, gray, format GGGGG.
    BFACE_PLANAR_U8C1_GRAY = 0x01,
    /// 1 channel, unsigned short, planar format, gray, format
    BFACE_PLANAR_U16C1_GRAY = 0x09,
    /// 3 channels, unsigned, 8 bits depth, planar format, bgr, format BBBGGGRRR.
    BFACE_PLANAR_U8C3_BGR = 0x0b,
    /// 3 channels, unsigned, 8 bits depth, semi planar format, yvu420sp format: YYYY UVUV.
    BFACE_PLANAR_U8C3_YVU420SP = 0x02,
    /// 3 channels, unsigned, 8 bits depth, planar format, yuv420p format: YYYY UU VV
    BFACE_PLANAR_U8C3_YUV420P = 0x4,
    ///  3 channels, unsigned, 8 bits depth, interleave format, bgr, format BGRBGRBGR
    BFACE_INTERLEAVE_U8C3_BGR = 0x10
} PIXEL_FORMAT;

/**
 * @brief enumerate of tracking status
 */
typedef enum {
    /// current target is tracked
    TRACKED = 0,
    /// current target is not tracked any more, the tracking ID is deleted by tracker
    DELETED = 1,
} TRACK_STATUS;

/**
 * @brief describe a rectangle area
 */
typedef struct {
    /// the x-coordinate of top-left point of a rectangle
    int left;
    /// the y-coordinate of top-left point of a rectangle
    int top;
    /// the width of a rectangle
    int width;
    /// the height of a rectangle
    int height;
} Rect_t;


/**
 * @brief image frame descriptor, bface_pointer_type is architecture relevant.
 */
struct Image_t {
    Image_t() {
        memset(this, 0, sizeof(Image_t));
    }
    /// release memory
    ~Image_t() = default;
    bface_pointer_type phy_addr[3];
    bface_pointer_type vir_addr[3];
    /// the stride of each image plane
    uint32_t stride[3];
    /// image width
    uint32_t width;
    /// image height
    uint32_t height;
    /// pixel format
    PIXEL_FORMAT pixel_format;
    /// timestamp, the timestamp when current frame is born
    uint64_t timestamp;
    /// from which video channel
    uint32_t channel_id;
    /// current index for its video channel.
    uint32_t frame_index;
    /// image type
    IMAGE_TYPE image_type;
};

typedef struct {
    float left;
    float top;
    float width;
    float height;
} Rectf_t;

/**
 * @brief describe a face landmark result.
 */
typedef struct {
    /// landmark index
    int index;
    /// landmark size, default value is 72
    int size;
    /// landmark coordinate
    std::vector<float> data;
    /// face confidence range from 0 to 1
    float score;
} Landmark_t;

/**
 * @brief Describe a bounding box
 */
typedef struct {
    /// the position of a bounding box within the image
    Rectf_t rect;
    /// confidence, output by a detection model
    float conf;
} BoundingBox_t;

typedef struct {
    BoundingBox_t bbox;
    int tracking_id;
    TRACK_STATUS tracking_status;
} TrackedBox_t;

/**
 * @brief describe pose of the face image
 */
typedef struct {
    /// pose angle degree value range from [-90, 90]
    float pitch;
    float roll;
    float yaw;
} Pose_t;

/**
 * @deprecated This type will be removed in future, please use RawFeatureId_t to replace it!
 * @brief describe a face feature with ID.
 */
typedef struct {
    int feature_id;
    /// face feature
    std::vector<float> feature;
} FeatureId_t;

using Byte_t = unsigned char;
/**
 * @brief describe a face feature with ID.
 */
struct RawFeatureId_t {
    int feature_id;
    /// face feature
    std::vector<Byte_t> feature;
};

/**
 * @brief describe a feature search result.
 * @see bface_search_feature
 */
typedef struct {
    /// the ID of current feature
    int feature_id;
    /// the confidence between the feature of feature_id and the searching feature
    /// range from 0 to 1
    float conf;
} SearchResult_t;

/**
 * @brief describe a basic dms result.
 */
typedef struct {
    float normal;
    float phone_calling;
    float drinking;
    float eating;
    float smoking;
} BasicDms_t;

/**
 * @brief describe a face attribute.
 */
typedef struct {
    float gender_score;
    float age_value;
} FaceAttr_t;

/**
 * @brief 用于表示每类过滤的状态，如果还没进行到此能力或正在调用中就是PENDING状态，此状态下一般结果数据为空，表示数据不可用，如果通过设定阈值
 * 就是PASSED状态，否则是FAILED状态
 */
enum class FilterStatus {
    PENDING,
    FAILED,
    PASSED
};

/**
 * @brief 用于表示每类能力中间结果回调方式，LIGHT=错误回调,HEAVY=每帧回调+错误回调
 */
enum class CallbackMethod {
    LIGHT,
    HEAVY
};

/**
 * @brief 用于表示每帧数据处理状态，RUNNING表示此帧数据正在Pipeline中处理，FINISHED表示此帧数据已经完成处理
 */
enum class FrameStatus {
    UNKNOWN,
    RUNNING,
    FINISHED
};

/// 表示pipeline中检测跟踪接口调用后的结果，包含详细数据及过滤判断
struct DetectResult {
    TrackedBox_t rgb_data;
    BoundingBox_t nir_data;
    FilterStatus rgb_status = FilterStatus::PENDING;
    FilterStatus nir_status = FilterStatus::PENDING;
};

/// 表示pipeline中最优人脸接口调用后的结果，包含详细数据及过滤判断
struct BestimgResult {
    float conf = 0;
    FilterStatus status = FilterStatus::PENDING;
};

/// 表示pipeline中对齐接口调用后的结果，包含详细数据及过滤判断
struct AlignResult {
    Landmark_t rgb_landmarks;
    Landmark_t nir_landmarks;
    FilterStatus rgb_status = FilterStatus::PENDING;
    FilterStatus nir_status = FilterStatus::PENDING;
};

/// 表示pipeline中综合质量（模糊+遮挡+姿态+光照）接口调用后的结果，包含详细数据及过滤判断
struct QualityResult {
    float blur = 0;
    std::vector<float> luminance = {0, 0};
    std::vector<float> occ_scores = std::vector<float>(7, 0.0);
    Pose_t pose = {0, 0, 0};
    FilterStatus status = FilterStatus::PENDING;
};

/// 表示pipeline中活体接口调用后的结果，包含详细数据及过滤判断，如果活体的判断结果不为UNKOWN表明，此人脸ID的活体最终判断结果输出
struct LivenessResult {
    float rgb_score = 0;
    float nir_score = 0;
    FilterStatus rgb_status = FilterStatus::PENDING;
    FilterStatus nir_status = FilterStatus::PENDING;
};

/// 表示最终提取的特征结果与数据状态
struct FeatureResult {
    std::vector<Byte_t> rgb_feature;
    FilterStatus rgb_status = FilterStatus::PENDING;
};

/// 表示pipeline中此次输出结果对应的视频帧信息，用于上层的信息同步
struct FrameInfo {
    /// from which video channel
    uint32_t channel_id = 0;
    /// current index for its video channel.
    uint32_t frame_index = 0;
    /// timestamp, the timestamp when current frame is born
    uint64_t timestamp = 0;
    /// frame status
    FrameStatus status = FrameStatus::UNKNOWN;
};

struct LivenessPipelineParam_t {
    struct CallbackParam {
        /// 检测结果回调方式
        CallbackMethod detect_callback = CallbackMethod::HEAVY;
        /// 最优人脸结果回调方式
        CallbackMethod bestimg_callback = CallbackMethod::LIGHT;
        /// 对齐结果回调方式
        CallbackMethod align_callback = CallbackMethod::LIGHT;
        /// 综合质量结果回调方式
        CallbackMethod quality_callback = CallbackMethod::LIGHT;
    };
    /// 人脸检测框最小的边框，默认最小人脸60x60
    int detect_border = 60;
    /// 人脸检测框的置信度阈值，范围[0-1]，值越大，说明检测框检测越准确
    float detect_threshold = 0.8;
    /// 人脸最优人脸质量分数阈值，范围[0-1]，数值越高，用于识别的人脸质量越好
    float bestimage_threshold = 0.8;
    /// 最优人脸固定窗口的大小
    int bestimage_window_size = 10;
    /// 人脸关键点的分数阈值，范围[0-1]，分值越高说明人脸可信度越高
    float align_threshold = 0.8;
    /// 人脸模糊度分数阈值，范围[0-1]，数值越高，说明人脸越清晰
    float blur_threshold = 0.8;
    /// 左眼\右眼\鼻子\嘴巴\左脸颊\右脸颊\下巴位置遮挡的置信度阈值，范围[0-1]，数值越高遮挡越轻
    std::vector<float> occlusion_threshold = {0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6};
    /// 单人脸角度检测，输出XYZ三个维度的欧拉角阈值，范围[-90 - 90]，角度绝对值的阈值推荐20度，当角度的绝对值小于这个阈值，认为是正人脸
    Pose_t pose_threshold = {20, 20, 20};
    /// 单个人脸光照暗度阈值，范围[0-1.0]，纯白为1，纯黑为0，越小说明人脸越暗，亮度应该在所设定范围内
    float darkness_threshold = 0.8;
    /// 单个人脸光照亮度阈值，范围[0-1.0]，纯白为0，纯黑为1，越小说明人脸越亮，亮度应该在所设定范围内
    float brightness_threshold = 0.8;
    /// rgb模态活体滑动窗口的大小
    int rgb_liveness_window_size = 10;
    /// rgb模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float rgb_liveness_threshold = 0.8;
    /// rgb模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float rgb_liveness_ratio = 0.8;
    /// nir模态活体滑动窗口的大小
    int nir_liveness_window_size = 10;
    /// nir模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float nir_liveness_threshold = 0.8;
    /// nir模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float nir_liveness_ratio = 0.8;
    /// 每类能力的中间数据结果上报方式
    CallbackParam callback_params;
};

/// 表示pipeline中某一帧输出的数据内容，可能包含其中几项或全部
struct LivenessPipelineResult_t {
    /// 视频帧相关信息
    FrameInfo frame_info;
    /// 检测跟踪中间结果
    DetectResult detect_result;
    /// 最优人脸中间结果
    BestimgResult bestimg_result;
    /// 对齐中间结果
    AlignResult align_result;
    /// 综合质量中间结果
    QualityResult quality_result;
    /// 活体最终结果
    LivenessResult liveness_result;
};
struct FeatureExtractPipelineParam_t {
    struct CallbackParam {
        /// 检测结果回调方式
        CallbackMethod detect_callback = CallbackMethod::LIGHT;
        /// 对齐结果回调方式
        CallbackMethod align_callback = CallbackMethod::LIGHT;
        /// 综合质量结果回调方式
        CallbackMethod quality_callback = CallbackMethod::LIGHT;
        /// 活体数据回调方式
        CallbackMethod liveness_callback = CallbackMethod::LIGHT;
    };
    /// 人脸检测框最小的边框，默认最小人脸60x60
    int detect_border = 60;
    /// 人脸检测框的置信度阈值，范围[0-1]，值越大，说明检测框检测越准确
    float detect_threshold = 0.8;
    /// 人脸关键点的分数阈值，范围[0-1]，分值越高说明人脸可信度越高
    float align_threshold = 0.8;
    /// 人脸模糊度分数阈值，范围[0-1]，数值越高，说明人脸越清晰
    float blur_threshold = 0.8;
    /// 左眼\右眼\鼻子\嘴巴\左脸颊\右脸颊\下巴位置遮挡的置信度阈值，范围[0-1]，数值越高遮挡越轻
    std::vector<float> occlusion_threshold = {0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6};
    /// 单人脸角度检测，输出XYZ三个维度的欧拉角阈值，范围[-90 - 90]，角度阈值推荐20度
    Pose_t pose_threshold = {20, 20, 20};
    /// 单个人脸光照暗度阈值，范围[0-1.0]，纯白为1，纯黑为0，越小说明人脸越暗，亮度应该在所设定范围内
    float darkness_threshold = 0.8;
    /// 单个人脸光照亮度阈值，范围[0-1.0]，纯白为0，纯黑为1，越小说明人脸越亮，亮度应该在所设定范围内
    float brightness_threshold = 0.8;
    /// rgb模态活体滑动窗口的大小
    int rgb_liveness_window_size = 10;
    /// rgb模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float rgb_liveness_threshold = 0.8;
    /// rgb模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float rgb_liveness_ratio = 0.8;
    /// nir模态活体滑动窗口的大小
    int nir_liveness_window_size = 10;
    /// nir模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float nir_liveness_threshold = 0.8;
    /// nir模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float nir_liveness_ratio = 0.8;
    /// 每类能力的中间数据结果上报方式
    CallbackParam callback_params;
};

/// 表示pipeline中某一帧输出的数据内容，可能包含其中几项或全部
struct FeatureExtractPipelineResult_t {
    /// 视频帧相关信息
    FrameInfo frame_info;
    /// 检测跟踪中间结果
    DetectResult detect_result;
    /// 对齐中间结果
    AlignResult align_result;
    /// 综合质量中间结果
    QualityResult quality_result;
    /// 活体中间结果
    LivenessResult liveness_result;
    /// 特征数据最终结果
    FeatureResult feature_result;
};

struct Feature1V1PipelineParam_t {
    struct CallbackParam {
        /// 检测结果回调方式
        CallbackMethod detect_callback = CallbackMethod::LIGHT;
        /// 最优人脸结果回调方式
        CallbackMethod bestimg_callback = CallbackMethod::LIGHT;
        /// 对齐结果回调方式
        CallbackMethod align_callback = CallbackMethod::LIGHT;
        /// 综合质量结果回调方式
        CallbackMethod quality_callback = CallbackMethod::LIGHT;
        /// 活体数据回调方式
        CallbackMethod liveness_callback = CallbackMethod::LIGHT;
        /// 特征数据回调方式
        CallbackMethod feature_callback = CallbackMethod::LIGHT;
    };
    /// 人脸检测框最小的边框，默认最小人脸60x60
    int detect_border = 60;
    /// 人脸检测框的置信度阈值，范围[0-1]，值越大，说明检测框检测越准确
    float detect_threshold = 0.8;
    /// 已经识别成功的人脸ID，重复识别时间间隔，单位ms
    int repeat_interval_time = 3000;
    /// 内部缓冲数据处理队列长度, 范围[10-30]
    int buffer_queue_length = 20;
    /// 人脸最优人脸质量分数阈值，范围[0-1]，数值越高，用于识别的人脸质量越好
    float bestimage_threshold = 0.8;
    /// 最优人脸固定窗口的大小
    int bestimage_window_size = 5;
    /// 人脸关键点的分数阈值，范围[0-1]，分值越高说明人脸可信度越高
    float align_threshold = 0.8;
    /// 人脸模糊度分数阈值，范围[0-1]，数值越高，说明人脸越清晰
    float blur_threshold = 0.8;
    /// 左眼\右眼\鼻子\嘴巴\左脸颊\右脸颊\下巴位置遮挡的置信度阈值，范围[0-1]，数值越高遮挡越轻
    std::vector<float> occlusion_threshold = {0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6};
    /// 单人脸角度检测，输出XYZ三个维度的欧拉角阈值，范围[-90 - 90]，角度阈值推荐20度
    Pose_t pose_threshold = {20, 20, 20};
    /// 单个人脸光照暗度阈值，范围[0-1.0]，纯白为1，纯黑为0，越小说明人脸越暗，亮度应该在所设定范围内
    float darkness_threshold = 0.8;
    /// 单个人脸光照亮度阈值，范围[0-1.0]，纯白为0，纯黑为1，越小说明人脸越亮，亮度应该在所设定范围内
    float brightness_threshold = 0.8;
    /// rgb模态活体滑动窗口的大小
    int rgb_liveness_window_size = 10;
    /// rgb模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float rgb_liveness_threshold = 0.8;
    /// rgb模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float rgb_liveness_ratio = 0.8;
    /// nir模态活体滑动窗口的大小
    int nir_liveness_window_size = 10;
    /// nir模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float nir_liveness_threshold = 0.8;
    /// nir模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float nir_liveness_ratio = 0.8;
    /// 特征底库类型
    StoreType compare_type = StoreType::RGB_ID_FEATURE;
    /// 特征相似度比对阈值，超过阈值才为真，范围[0-1]
    float compare_thr = 0.8;
    /// 相同ID比对失败情况下最多比对次数
    int compare_times = 5;
    /// 每类能力的中间数据结果上报方式
    CallbackParam callback_params;
};

/// 接口输出的最终人证比对结果及数据状态
struct CompareResult {
    /// 比对相似度
    float similarity;
    /// 生活照特征值
    std::vector<Byte_t> living_feature;
    /// 证件照特征值
    std::vector<Byte_t> credit_feature;
    /// 结果状态
    FilterStatus status = FilterStatus::PENDING;;
};

/// 表示pipeline中某一帧输出的数据内容，可能包含其中几项或全部
struct Feature1V1PipelineResult_t {
    FrameInfo frame_info;
    DetectResult detect_result;
    BestimgResult bestimg_result;
    AlignResult align_result;
    QualityResult quality_result;
    LivenessResult liveness_result;
    CompareResult compare_result;
};

struct Feature1VnPipelineParam_t {
    struct CallbackParam {
        /// 检测结果回调方式
        CallbackMethod detect_callback = CallbackMethod::LIGHT;
        /// 最优人脸结果回调方式
        CallbackMethod bestimg_callback = CallbackMethod::LIGHT;
        /// 对齐结果回调方式
        CallbackMethod align_callback = CallbackMethod::LIGHT;
        /// 综合质量结果回调方式
        CallbackMethod quality_callback = CallbackMethod::LIGHT;
        /// 活体数据回调方式
        CallbackMethod liveness_callback = CallbackMethod::LIGHT;
        /// 特征数据回调方式
        CallbackMethod feature_callback = CallbackMethod::LIGHT;
    };
    /// 人脸检测框最小的边框，默认最小人脸60x60
    int detect_border = 60;
    /// 人脸检测框的置信度阈值，范围[0-1]，值越大，说明检测框检测越准确
    float detect_threshold = 0.8;
    /// 已经识别成功的人脸ID，重复识别时间间隔，单位ms
    int repeat_interval_time = 3000;
    /// 内部缓冲数据处理队列长度, 范围[10-30]
    int buffer_queue_length = 20;
    /// 人脸最优人脸质量分数阈值，范围[0-1]，数值越高，用于识别的人脸质量越好
    float bestimage_threshold = 0.8;
    /// 最优人脸固定窗口的大小
    int bestimage_window_size = 5;
    /// 人脸关键点的分数阈值，范围[0-1]，分值越高说明人脸可信度越高
    float align_threshold = 0.8;
    /// 人脸模糊度分数阈值，范围[0-1]，数值越高，说明人脸越清晰
    float blur_threshold = 0.8;
    /// 左眼\右眼\鼻子\嘴巴\左脸颊\右脸颊\下巴位置遮挡的置信度阈值，范围[0-1]，数值越高遮挡越轻
    std::vector<float> occlusion_threshold = {0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6};
    /// 单人脸角度检测，输出XYZ三个维度的欧拉角阈值，范围[-90 - 90]，角度阈值推荐20度
    Pose_t pose_threshold = {20, 20, 20};
    /// 单个人脸光照暗度阈值，范围[0-1.0]，纯白为1，纯黑为0，越小说明人脸越暗，亮度应该在所设定范围内
    float darkness_threshold = 0.8;
    /// 单个人脸光照亮度阈值，范围[0-1.0]，纯白为0，纯黑为1，越小说明人脸越亮，亮度应该在所设定范围内
    float brightness_threshold = 0.8;
    /// rgb模态活体滑动窗口的大小
    int rgb_liveness_window_size = 10;
    /// rgb模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float rgb_liveness_threshold = 0.8;
    /// rgb模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float rgb_liveness_ratio = 0.8;
    /// nir模态活体滑动窗口的大小
    int nir_liveness_window_size = 10;
    /// nir模态，单个人脸的活体分数阈值，范围[0-1]，分值越高说明人脸约可能是活体
    float nir_liveness_threshold = 0.8;
    /// nir模态，窗口内活体分数超过阈值比例，范围[0-1]，分值越高说明活体结果所占比例越大
    float nir_liveness_ratio = 0.8;
    /// 特征底库类型
    StoreType search_type = StoreType::RGB_FEATURE;
    /// 特征检索阈值，超过阈值才为真，范围[0-1]
    float search_thr = 0.8;
    /// 特征检索TOP数量
    float search_num = 3;
    /// 相同ID比对失败情况下最多比对次数
    int compare_times = 5;
    /// 每类能力的中间数据结果上报方式
    CallbackParam callback_params;
};

/// 接口输出的最终特征库检索结果及数据状态
struct SearchResult {
    /// 检索结果
    std::vector<SearchResult_t> results;
    /// TOP1检索比对结果
    bool is_passed;
    /// 结果状态
    FilterStatus status = FilterStatus::PENDING;;
};

/// 表示pipeline中某一帧输出的数据内容，可能包含其中几项或全部
struct Feature1VnPipelineResult_t {
    FrameInfo frame_info;
    DetectResult detect_result;
    BestimgResult bestimg_result;
    AlignResult align_result;
    QualityResult quality_result;
    LivenessResult liveness_result;
    SearchResult search_result;
};

/// smart image construct params (new Image_t, ImageDeleter())
using SmartImage_t = std::shared_ptr<bface::Image_t>;

using LivenessPipelineCallback_t =
    std::function<void(const LivenessPipelineResult_t)>;

using FeatureExtractPipelineCallback_t =
    std::function<void(const FeatureExtractPipelineResult_t)>;

using Feature1V1PipelineCallback_t =
    std::function<void(const Feature1V1PipelineResult_t)>;

using Feature1VnPipelineCallback_t =
    std::function<void(const Feature1VnPipelineResult_t)>;

}  /// namespace bface