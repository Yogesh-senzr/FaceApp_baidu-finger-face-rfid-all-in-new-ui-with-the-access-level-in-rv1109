#ifndef BFACE_DEFAULT_PARAM_H
#define BFACE_DEFAULT_PARAM_H
#include <interface/bface_export.h>
#include <cstdint>
#include <cstring>
#include <vector>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief the root namespace
 */
namespace bface {
// @brief baidu hisi SDK default param list

#define BD_TRACKING_RGB_CONF     0.5
#define BD_TRACKING_NIR_CONF     0.5
// liveness Pass thresold
#define BD_LIVENESS_RGB_LOW      0.1
#define BD_LIVENESS_RGB_NORMAL   0.5
#define BD_LIVENESS_RGB_HIGH     0.9

#define BD_LIVENESS_NIR_LOW      0.1
#define BD_LIVENESS_NIR_NORMAL   0.5
#define BD_LIVENESS_NIR_HIGH     0.9

#define BD_LIVENESS_DEPTH_LOW    0.1
#define BD_LIVENESS_DEPTH_NORMAL 0.5
#define BD_LIVENESS_DEPTH_HIGH   0.9

// Poss, value must be less than thresold
#define BD_QUALITY_POSE_LOW     30
#define BD_QUALITY_POSE_NORMAL  20
#define BD_QUALITY_POSE_HIGH    15

// occlusion value must be less than the thresold
#define BD_QUALITY_OCCLUSION_LOW     0.8
#define BD_QUALITY_OCCLUSION_NORMAL  0.6
#define BD_QUALITY_OCCLUSION_HIGH    0.3
#define BD_QUALITY_OCCLUSION_RECOMMEND 0.9

// blur vlaue must be less than the thresold
#define BD_QUALITY_BLUR_LOW      0.8
#define BD_QUALITY_BLUR_NORMAL   0.6
#define BD_QUALITY_BLUR_HIGH     0.3

// luminance vlaue must be in this range
#define BD_QUALITY_LUMINANCE_LOW_MIN     1
#define BD_QUALITY_LUMINANCE_LOW_MAX     255
#define BD_QUALITY_LUMINANCE_NORMAL_MIN  40
#define BD_QUALITY_LUMINANCE_NORMAL_MAX  200
#define BD_QUALITY_LUMINANCE_HIGH_MIN    60
#define BD_QUALITY_LUMINANCE_HIGH_MAX    120

// recognition feature compare thresold, more than 
#define BD_RECOG_COMPARE_LOW    0.7
#define BD_RECOG_COMPARE_NORMAL 0.8
#define BD_RECOG_COMPARE_HIGH   0.9
}  // namespace bface

#ifdef __cplusplus
}
#endif

#endif  // BFACE_DEFAULT_PARAM_H
