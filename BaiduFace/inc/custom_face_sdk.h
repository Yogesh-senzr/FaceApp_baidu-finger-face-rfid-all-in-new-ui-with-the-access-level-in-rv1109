#ifndef CUSTOM_FACE_SDK_H
#define CUSTOM_FACE_SDK_H

#include "interface/bface_types.h"
#include "interface/faceid_interface.h"

namespace custom_face {

using namespace bface;

// Initialize the SDK
BFACE_STATUS custom_sdk_init(const char* model_path = "/isc/models");

// Uninitialize the SDK
BFACE_STATUS custom_sdk_uninit();

// Face detection and tracking
BFACE_STATUS custom_detect_and_track(const Image_t& image, 
                                   std::vector<TrackedBox_t>* tracked_faces);

// Face quality score
BFACE_STATUS custom_quality_score(const Image_t& image,
                                const BoundingBox_t& face,
                                float* quality_score);

} // namespace custom_face

#endif // CUSTOM_FACE_SDK_H
