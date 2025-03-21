#ifndef CUSTOM_FACE_INTERFACE_H
#define CUSTOM_FACE_INTERFACE_H

#include "bface_types.h"
#include "faceid_interface.h"
#include "retinaface_detector.h"
#include <memory>

namespace custom_face {

class CustomFaceInterface {
public:
    static CustomFaceInterface* getInstance();
    
    BFACE_STATUS init(const char* model_path = "/isc/models");
    BFACE_STATUS uninit();
    
    BFACE_STATUS detect_and_track(const Image_t& image,
                                std::vector<TrackedBox_t>* tracked_faces);
    
    BFACE_STATUS quality_score(const Image_t& image,
                             const BoundingBox_t& face,
                             float* quality_score);

private:
    CustomFaceInterface();
    ~CustomFaceInterface();
    
    cv::Mat convertToCvMat(const Image_t& image);
    TrackedBox_t convertToTrackedBox(const cv::Rect& rect, int track_id, float score);
    
    static CustomFaceInterface* instance;
    std::unique_ptr<RetinafaceDetector> detector;
};

} // namespace custom_face

#endif // CUSTOM_FACE_INTERFACE_H
