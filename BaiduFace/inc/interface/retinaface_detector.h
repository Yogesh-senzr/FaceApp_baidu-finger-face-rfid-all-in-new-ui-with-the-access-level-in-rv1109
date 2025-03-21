#ifndef RETINAFACE_DETECTOR_H
#define RETINAFACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include <vector>
#include <memory>
#include <string>

class RetinafaceDetector {
public:
    // Result structures
    struct DetectionResult {
        std::vector<cv::Rect> boxes;
        std::vector<float> scores;
        std::vector<std::vector<cv::Point2f>> landmarks;
    };

    struct TrackingResult {
        std::vector<cv::Rect> boxes;
        std::vector<int> track_ids;
        std::vector<float> scores;
    };

    struct LandmarkResult {
        std::vector<cv::Point2f> points;
        bool success;
    };

    RetinafaceDetector();
    ~RetinafaceDetector();

    bool init();
    DetectionResult detectFaces(const cv::Mat& image, float conf_threshold = 0.5f);
    TrackingResult detectAndTrack(const cv::Mat& image, float conf_threshold = 0.5f);
    LandmarkResult getFacialLandmarks(const cv::Mat& image, const cv::Rect& face);

private:
    // Model parameters
    const std::string MODEL_PATH = "model/retinaface-resnet50.onnx";
    std::unique_ptr<Ort::Session> session;
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING};
    std::vector<const char*> input_names;
    std::vector<const char*> output_names;

    // Tracking state
    struct TrackedFace {
        int track_id;
        cv::Rect box;
        cv::KalmanFilter kalman;
        int missing_frames;
        float score;
    };
    std::vector<TrackedFace> tracked_faces;
    int next_track_id = 0;

    // Model parameters
    const int input_size = 640;
    std::vector<std::vector<float>> anchor_base;
    std::vector<float> anchor_scales;

    // Helper functions
    cv::Mat preprocess(const cv::Mat& img);
    std::vector<float> runInference(const cv::Mat& preprocessed);
    DetectionResult postprocess(const std::vector<float>& net_output,float conf_threshold,const cv::Size& orig_size);
    void updateTracks(const DetectionResult& detections);
    float calculateIOU(const cv::Rect& box1, const cv::Rect& box2);
    void initializeAnchors();
};

#endif