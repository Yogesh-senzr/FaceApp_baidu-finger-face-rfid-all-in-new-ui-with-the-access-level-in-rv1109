#include "custom_face_sdk.h"
#include "interface/custom_face_interface.h"
#include "MessageHandler/Log.h"

namespace custom_face {

CustomFaceInterface* CustomFaceInterface::instance = nullptr;

CustomFaceInterface* CustomFaceInterface::getInstance() {
    if (!instance) {
        instance = new CustomFaceInterface();
    }
    return instance;
}

CustomFaceInterface::CustomFaceInterface() 
    : detector(std::make_unique<RetinafaceDetector>()) {
}

CustomFaceInterface::~CustomFaceInterface() {
}

cv::Mat CustomFaceInterface::convertToCvMat(const Image_t& image) {
    // Convert BGR888 format to cv::Mat
    return cv::Mat(image.height, image.width, CV_8UC3, 
                  (void*)image.vir_addr[0]);
}

TrackedBox_t CustomFaceInterface::convertToTrackedBox(
    const cv::Rect& rect, int track_id, float score) {
    
    TrackedBox_t tracked_box;
    tracked_box.bbox.rect.left = rect.x;
    tracked_box.bbox.rect.top = rect.y;
    tracked_box.bbox.rect.width = rect.width;
    tracked_box.bbox.rect.height = rect.height;
    tracked_box.bbox.conf = score;
    tracked_box.tracking_id = track_id;
    tracked_box.tracking_status = TRACKED;
    
    return tracked_box;
}

BFACE_STATUS CustomFaceInterface::init(const char* model_path) {
    try {
        if (detector->init()) {
            LogD("Custom face detector initialized successfully\n");
            return BFACE_SUCCESS;
        }
        LogE("Failed to initialize custom face detector\n");
        return BFACE_FAILED;
    }
    catch (const std::exception& e) {
        LogE("Exception during initialization: %s\n", e.what());
        return BFACE_FAILED;
    }
}

BFACE_STATUS CustomFaceInterface::detect_and_track(
    const Image_t& image,
    std::vector<TrackedBox_t>* tracked_faces) {
    
    try {
        cv::Mat cv_image = convertToCvMat(image);
        
        auto result = detector->detectAndTrack(cv_image);
        
        tracked_faces->clear();
        for (size_t i = 0; i < result.boxes.size(); i++) {
            tracked_faces->push_back(
                convertToTrackedBox(
                    result.boxes[i],
                    result.track_ids[i],
                    result.scores[i]
                )
            );
        }
        
        return BFACE_SUCCESS;
    }
    catch (const std::exception& e) {
        LogE("Detection error: %s\n", e.what());
        return BFACE_FAILED;
    }
}

BFACE_STATUS CustomFaceInterface::quality_score(
    const Image_t& image,
    const BoundingBox_t& face,
    float* quality_score) {
    
    try {
        cv::Mat cv_image = convertToCvMat(image);
        cv::Rect face_rect(
            face.rect.left,
            face.rect.top,
            face.rect.width,
            face.rect.height
        );
        
        auto landmarks = detector->getFacialLandmarks(cv_image, face_rect);
        if (landmarks.success) {
            // Simple quality score based on face size and position
            float relative_size = (float)(face_rect.width * face_rect.height) / 
                                (cv_image.rows * cv_image.cols);
            *quality_score = relative_size > 0.01 ? 0.9f : 0.3f;
            return BFACE_SUCCESS;
        }
        
        *quality_score = 0.0f;
        return BFACE_FAILED;
    }
    catch (const std::exception& e) {
        LogE("Quality score error: %s\n", e.what());
        return BFACE_FAILED;
    }
}

// SDK Function Implementations
BFACE_STATUS custom_sdk_init(const char* model_path) {
    return CustomFaceInterface::getInstance()->init(model_path);
}

BFACE_STATUS custom_sdk_uninit() {
    if (CustomFaceInterface::instance) {
        delete CustomFaceInterface::instance;
        CustomFaceInterface::instance = nullptr;
    }
    return BFACE_SUCCESS;
}

BFACE_STATUS custom_detect_and_track(
    const Image_t& image,
    std::vector<TrackedBox_t>* tracked_faces) {
    return CustomFaceInterface::getInstance()->detect_and_track(
        image, tracked_faces);
}

BFACE_STATUS custom_quality_score(
    const Image_t& image,
    const BoundingBox_t& face,
    float* quality_score) {
    return CustomFaceInterface::getInstance()->quality_score(
        image, face, quality_score);
}

} // namespace custom_face
