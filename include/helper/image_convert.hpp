#ifndef _IMAGE_CONVERT_HPP
#define _IMAGE_CONVERT_HPP
#include "opencv2/opencv.hpp"

#include "interface/bface_export.h"
#include "interface/bface_types.h"

namespace bface {
namespace helper {
/**
 * @brief convert cvMat to Image_t
 * @param mat input cvMat
 * @param img_ptr output Image_t
 * @param used_mmz whether to use mmz memory for img_ptr
 * @param align32 whether to allocate image buffer with stride 32 bytes aligned
 * @return API status code
 */
BFACE_API
BFACE_STATUS convert_cvmat_to_image_t(const cv::Mat& mat, Image_t* img_ptr, bool used_mmz = false, bool align32 = false);

/**
 * @brief convert Image_t to cvMat, the stride is considered internally, 
 *        BGR_INTERLEAVE BGR_PLANNAR and YVU420SP is supported
 * @param img input image
 * @param mat_ptr
 * @return API status code
 */
BFACE_API
BFACE_STATUS convert_image_t_to_cvmat(const Image_t& img, cv::Mat* mat_ptr);

/**
 * @brief convert cvMat to Image_t with square resolution
 * @param mat input cvMat
 * @param img_ptr output Image_t
 * @return API status code
 */
// TODO(zhourenyi) align to convert_cvmat_to_image_t with used_mmz and align32
BFACE_API
BFACE_STATUS convert_cvmat_to_image_t_square(const cv::Mat& mat, Image_t* img_ptr);

/**
 * @brief image affine transformation
 * @param src the input image
 * @param m transformation matrix, src->dst
 * @param dst_ptr output image
 * @param size the size of output image
 */
BFACE_API
void warp_affine(const cv::Mat& src, const cv::Mat& m, cv::Mat* dst_ptr, cv::Size size);

/**
 * @brief cvMat image convert with align ratio resolution
 * @param src_img the input mat
 * @param mat_ptr the output mat pointer
 * @param min_width align resolution width, after resize width
 * @param min_height align resolution height, after resize height
 */
BFACE_API
BFACE_STATUS convert_cvmat_with_ratio(const cv::Mat& src_img, cv::Mat* mat_ptr,
                                      const int min_width, const int min_height);

/**
 * @brief read yuv image and convert yuv image to cvmat with rgb format
 * @param[in] yuv_filename yuv fomat image file
 * @param[in] yuv_size yuv image size
 * @param[out] rgb_cvmat yuv format image convert rgb cvmat
 * @return BFACE_SUCCESS if success
 */
BFACE_API
BFACE_STATUS read_yuv_file_convert_cvmat(std::string yuv_filename, cv::Size yuv_size, cv::Mat& rgb_cvmat);

}  /// namespace helper
}  /// namespace bface
#endif  // _IMG_UTILS_HPP
