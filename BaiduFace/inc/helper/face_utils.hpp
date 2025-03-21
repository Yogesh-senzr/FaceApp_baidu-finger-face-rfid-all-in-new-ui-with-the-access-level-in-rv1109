#ifndef _FACE_UTILS_HPP
#define _FACE_UTILS_HPP
#include <interface/bface_types.h>
#include <interface/bface_export.h>
#include <memory>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>

namespace bface {
namespace helper {
typedef enum { BIDIRECTION, HORIZONAL, VERTICAL, PERSPTRANS } DIRECTION_TYPE;
typedef enum { ONLY_BBOX, LMK_BBOX} SPLIT_TYPE;

/**
 * @brief 用于活体辅助判断的接口类，主要用于过滤偶现攻击
 */
class ILiveJudgmentTool {
public:
    ILiveJudgmentTool() = default;
    virtual ~ILiveJudgmentTool() = default;
    /**
     * @brief 检测活体分值是否通过
     * @param score 当前活体分值
     * @param id 当前人脸id（通常为跟踪id）
     * @return true 通过
     * @return false 未通过
     */
    virtual bool check(float score, int id) = 0;
    /**
     * @brief 获取内部信息，仅用于开发者调试，用户可忽略
     * @return std::string 实时内部信息
     */
    virtual std::string get_info() = 0;
};

/**
 * @brief 创建活体辅助判断工具（建议：创建时机为sdk初始化，结束后会自动释放）
 * @param max_len 最大缓存长度，默认为10
 * @param score_thres 单个活体分值阈值，默认为0.8
 * @param len_thres 多个活体占比判断阈值，默认为0.8
 * @param min_valid_frame_num 最小通过数量，默认为0，范围[0~max_len]
 * @return 非空指针：成功；空指针：失败 
 */
BFACE_API
std::shared_ptr<ILiveJudgmentTool> create_live_judgment_tool(
        int max_len = 10, 
        float score_thres = 0.8f, 
        float len_thres = 0.8f, 
        int min_valid_frame_num = 0);

/**
 * @brief align the image resolution with specified step.
 * @param img image to be aligned
 * @param step align step
 * @return true if legal, false if illegal
 */
static bool algin_image_size(cv::Mat& img, int step) {
    if (img.empty()) {
        std::cerr << "input mat is empty!" << std::endl;
        return false;
    }
    if ((img.cols % step == 0) && (img.rows % step == 0)) {
        return true;
    }
    int dst_w = (int(img.cols / step) + 1) * step;
    int dst_h = (int(img.rows / step) + 1) * step;
    cv::Mat dst_mat(dst_h, dst_w, img.type());
    cv::Mat roi_mat = dst_mat(cv::Rect(0, 0, img.cols, img.rows));
    img.copyTo(roi_mat);
    img = dst_mat;
    return true;
}

/**
 * @brief check whether image resolution is legal, i.e. no bigger than 1920x1080 in horizonal
 *        shape or 1080x1920 in vertical shape
 * @param img image to be checked
 * @param type  check type
 * @return true if legal, false if illegal
 */
BFACE_API
bool check_image_size(cv::Mat& img, DIRECTION_TYPE type = BIDIRECTION);
/**
 * @brief bbox convert with scale
 * @param rect the input rect
 * @param rect_ptr the output rect pointer
 * @param scale scale value
 * @param width image width
 * @param height image height
 */
BFACE_API
BFACE_STATUS convert_bbox_with_scale(const Rectf_t rect, Rectf_t* rect_ptr, const float scale,
                                     const uint32_t width, uint32_t height);
/**
 * @brief dump landmark with scale
 * @param landmark_results input face landmark result
 * @param width_scale input width scale
 * @param height_scale input height scale
 */
BFACE_API
void dump_landmark(Landmark_t& landmark_results, float width_scale, float height_scale);

/**
 * @brief plot landmark on image
 * @param landmark_results input face landmark result
 * @param image input image
 */
BFACE_API
cv::Mat plot_landmark(const Landmark_t& landmark_results, cv::Mat image);

/**
 * @brief split image name from input string and delimiting char
 * @param str input string
 * @param delim input delimiting char
 * @param name_ptr output the splited name pointer
 */
BFACE_API
bool split_name(std::string str, const char *delim, std::string* name_ptr);

/**
 * @brief split prefix from input string and delimiting char
 * @param str input string
 * @param delim input delimiting char
 * @param prefix_ptr output the splited name pointer
 */
BFACE_API
bool split_prefix_from_name(std::string str, const char *delim, std::string* prefix_ptr);

/**
 * @brief split label from image name by delimiting char
 * @param str input string
 * @param delim input delimiting char
 * @param label_ptr output the splited label pointer
 */
BFACE_API
bool split_label_from_name(std::string str, const char *delim, std::string* label_ptr);
/**
 * @brief split landmark and image name from string
 * @param line input one string line
 * @param img_path_ptr output image name pointer
 * @param ldmk_ptr output face landmark pointer
 */
BFACE_API
bool split_name_and_landmarks(std::string line, std::string* img_path_ptr, Landmark_t* ldmk_ptr);

/**
 * @brief split bbox and image name from string
 * @param line input string
 * @param name_ptr output image name pointer
 * @param bbox_ptr output face bbox pointer
 */
BFACE_API
bool split_name_and_bbox(const std::string& line, std::string* name_ptr, BoundingBox_t* bbox_ptr, SPLIT_TYPE type);

/**
 * @brief plot bbox on image
 * @param bbox input face bbox
 * @param image input image
 */
BFACE_API
cv::Mat plot_bbox(const BoundingBox_t& bbox, cv::Mat image);

/**
 * @brief convert landmark by scale
 * @param input_landmark input face landmark result
 * @param output_landmark_ptr output face landmark pointer
 * @param width_scale input width scale
 * @param height_scale input height scale
 */
BFACE_API
void convert_landmark_with_scale(const Landmark_t& input_landmark, Landmark_t* output_landmark_ptr,
                                 const float width_scale, const float height_scale);

/**
 * @brief transmit the coordinate of a point from old CSYS to new CSYS
 * @param[in] sx the x-axis coordinate in old CSYS
 * @param[in] sy the y-axis coordinate in old CSYS
 * @param[in] ox the old CSYS origin point's x-axis coordinate in new CSYS
 * @param[in] oy the old CSYS origin point's y-axis coordinate in new CSYS
 * @param[out] sx the x-axis coordinate in new CSYS
 * @param[out] sy the y-axis coordinate in new CSYS
 */
template <typename T>
BFACE_API void trans_to_new_csys(T& sx, T& sy, T ox, T oy) {
    sx += ox;
    sy += oy;
}

/**
 * @brief transmit the coordinate of a point from old CSYS to new CSYS
 * @param point the coordinate in old CSYS as input, and coordinate in new CSYS as output
 * @param origin the old CSYS origin point's coordinate in new CSYS
 */
template <typename T>
BFACE_API void trans_point_to_new_csys(T& point, T origin) {
    trans_to_new_csys(point.x, point.y, origin.x, origin.y);
}

/**
 * @brief get face bounding box from old landmarks
 * @param ldmks face landmarks
 * @param bbox face bounding_box
 * @param img_width image max width
 * @param img_height image max height
 */
bool get_bounding_box(const Landmark_t& ldmks, BoundingBox_t* bbox, float img_width, float img_height);

/**
 * @brief get new bbox that the forehead area dropped by landmarks
 * @param[in] ldmk the landmarks
 * @param[out] bbox new bbox whose forehead area is cut
 */
void cut_forehead_area(Landmark_t& ldmk, BoundingBox_t* bbox);

/**
 * @brief get face bounding box from old landmarks
 * @param img face image of Image_t format
 * @param bbox face bounding_box
 * @param value face illumination value
 */
bool calc_face_illumination(const Image_t &img, const BoundingBox_t& bbox, int* value);

/**
 * @brief get face bounding box from old landmarks
 * @param img_mat face image of cv::mat
 * @param bbox face bounding box
 * @param value face illumination value
 */
bool calc_face_illumination(const cv::Mat img_mat, const BoundingBox_t& bbox, int* value);

/**
 * @brief make sure bbox and roi is overlap
 * @param bbox bounding box
 * @param roi roi bounding box
 */
bool ROI_bbox_overlap(const BoundingBox_t& bbox, const Rectf_t& roi);

/**
 * @brief dump src image with circle landmarks
 * @param src_img dump image
 * @param mul_lmks multi-landmarks
 * @param rel_name relative file name
 */
bool dump_picture_with_multi_lmks(const Image_t& src_img,
                                  const std::vector<float*> mul_lmks,
                                  const std::string& rel_name);

/**
 * @brief dump src image
 * @param src_img dump image
 * @param rel_name relative file name
 */
bool dump_picture(const Image_t& src_img, const std::string& rel_name);

}  // namespace helper
}  /// namespace bface
#endif  // _IMG_UTILS_HPP
