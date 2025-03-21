#include "retinaface_detector.h"
#include <fstream>

RetinafaceDetector::RetinafaceDetector() {}

RetinafaceDetector::~RetinafaceDetector() {}

bool RetinafaceDetector::init() {
    try {
        // Check if model file exists
        std::ifstream f(MODEL_PATH.c_str());
        if (!f.good()) {
            std::cerr << "Model file not found at: " << MODEL_PATH << std::endl;
            return false;
        }

        // Create session
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        session = std::make_unique<Ort::Session>(env, MODEL_PATH.c_str(), session_options);

        // Setup input/output names
        input_names = {"input"};
        output_names = {"boxes", "scores", "landmarks"};

        // Initialize anchor generation
        initializeAnchors();

        std::cout << "Model loaded successfully from: " << MODEL_PATH << std::endl;
        return true;
    }
    catch (const Ort::Exception& e) {
        std::cerr << "ONNX initialization failed: " << e.what() << std::endl;
        return false;
    }
}

RetinafaceDetector::DetectionResult RetinafaceDetector::detectFaces(
        const cv::Mat& image, float conf_threshold) {

    // Preprocess image
    cv::Mat preprocessed = preprocess(image);

    // Run inference
    std::vector<float> net_output = runInference(preprocessed);

    // Post-process results
    return postprocess(net_output, conf_threshold, image.size());
}

RetinafaceDetector::TrackingResult RetinafaceDetector::detectAndTrack(
        const cv::Mat& image, float conf_threshold) {

    TrackingResult result;

    // Get detections
    DetectionResult detections = detectFaces(image, conf_threshold);

    // Update tracking
    updateTracks(detections);

    // Prepare tracking result
    for (const auto& track : tracked_faces) {
        if (track.missing_frames == 0) {
            result.boxes.push_back(track.box);
            result.track_ids.push_back(track.track_id);
            result.scores.push_back(track.score);
        }
    }

    return result;
}

RetinafaceDetector::LandmarkResult RetinafaceDetector::getFacialLandmarks(
        const cv::Mat& image, const cv::Rect& face) {

    LandmarkResult result;
    result.success = false;

    // Get detections with landmarks
    DetectionResult detections = detectFaces(image, 0.5f);

    // Find matching face
    float best_iou = 0.0f;
    size_t best_idx = 0;
    bool found = false;

    for (size_t i = 0; i < detections.boxes.size(); i++) {
        float iou = calculateIOU(face, detections.boxes[i]);
        if (iou > best_iou) {
            best_iou = iou;
            best_idx = i;
            found = true;
        }
    }

    if (found && best_iou > 0.5f) {
        result.points = detections.landmarks[best_idx];
        result.success = true;
    }

    return result;
}

cv::Mat RetinafaceDetector::preprocess(const cv::Mat& img) {
    cv::Mat resized;
    float scale = static_cast<float>(input_size) / std::max(img.cols, img.rows);
    cv::resize(img, resized, cv::Size(), scale, scale);

    // Convert to float and normalize
    cv::Mat float_img;
    resized.convertTo(float_img, CV_32F);
    float_img = float_img / 255.0f;

    // Normalize with ImageNet mean and std
    cv::Scalar mean(0.485, 0.456, 0.406);
    cv::Scalar stddev(0.229, 0.224, 0.225);
    cv::subtract(float_img, mean, float_img);
    cv::divide(float_img, stddev, float_img);

    return float_img;
}

std::vector<float> RetinafaceDetector::runInference(const cv::Mat& preprocessed) {
    // Prepare input tensor
    std::vector<float> input_tensor = preprocessed.isContinuous() ?
                                      preprocessed.reshape(1, 1) : preprocessed.clone().reshape(1, 1);

    std::vector<int64_t> input_shape = {1, 3, input_size, input_size};

    auto memory_info = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor_ort = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor.data(), input_tensor.size(),
            input_shape.data(), input_shape.size());

    // Run inference
    auto output_tensors = session->Run(
            Ort::RunOptions{nullptr},
            input_names.data(),
            &input_tensor_ort,
            1,
            output_names.data(),
            output_names.size());

    // Get output data
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    size_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

    return std::vector<float>(output_data, output_data + output_size);
}

RetinafaceDetector::DetectionResult RetinafaceDetector::postprocess(
        const std::vector<float>& net_output,
        float conf_threshold,
        const cv::Size& orig_size) {

    DetectionResult result;

    // Convert network output to boxes, scores, and landmarks
    // This is a simplified version - you'll need to adapt based on your model's output format
    size_t box_step = 4;  // x, y, width, height
    size_t landmark_step = 10;  // 5 landmarks with x,y coordinates each

    for (size_t i = 0; i < net_output.size(); i += (box_step + 1 + landmark_step)) {
        if (net_output[i + 4] >= conf_threshold) {
            // Add box
            cv::Rect box;
            box.x = net_output[i] * orig_size.width;
            box.y = net_output[i + 1] * orig_size.height;
            box.width = (net_output[i + 2] - net_output[i]) * orig_size.width;
            box.height = (net_output[i + 3] - net_output[i + 1]) * orig_size.height;

            // Add score
            float score = net_output[i + 4];

            // Add landmarks
            std::vector<cv::Point2f> landmarks;
            for (size_t j = 0; j < 5; j++) {
                float x = net_output[i + 5 + j*2] * orig_size.width;
                float y = net_output[i + 5 + j*2 + 1] * orig_size.height;
                landmarks.push_back(cv::Point2f(x, y));
            }

            result.boxes.push_back(box);
            result.scores.push_back(score);
            result.landmarks.push_back(landmarks);
        }
    }

    return result;
}

void RetinafaceDetector::updateTracks(const DetectionResult& detections) {
    // Predict new locations for existing tracks
    for (auto& track : tracked_faces) {
        cv::Mat prediction = track.kalman.predict();
        track.box.x = prediction.at<float>(0);
        track.box.y = prediction.at<float>(1);
        track.box.width = prediction.at<float>(4);
        track.box.height = prediction.at<float>(5);
    }

    // Match detections to tracks
    std::vector<bool> matched_detections(detections.boxes.size(), false);

    for (auto& track : tracked_faces) {
        float best_iou = 0.3f;
        int best_match = -1;

        for (size_t i = 0; i < detections.boxes.size(); i++) {
            if (!matched_detections[i]) {
                float iou = calculateIOU(track.box, detections.boxes[i]);
                if (iou > best_iou) {
                    best_iou = iou;
                    best_match = i;
                }
            }
        }

        if (best_match >= 0) {
            // Update track
            track.box = detections.boxes[best_match];
            track.score = detections.scores[best_match];
            track.missing_frames = 0;
            matched_detections[best_match] = true;

            // Update Kalman filter
            cv::Mat measurement = (cv::Mat_<float>(6,1) <<
                                                        track.box.x, track.box.y, 0, 0,
                    track.box.width, track.box.height);
            track.kalman.correct(measurement);
        } else {
            track.missing_frames++;
        }
    }

    // Create new tracks
    for (size_t i = 0; i < detections.boxes.size(); i++) {
        if (!matched_detections[i]) {
            TrackedFace new_track;
            new_track.track_id = next_track_id++;
            new_track.box = detections.boxes[i];
            new_track.score = detections.scores[i];
            new_track.missing_frames = 0;

            // Initialize Kalman filter
            new_track.kalman.init(6, 6, 0);
            new_track.kalman.transitionMatrix = (cv::Mat_<float>(6,6) <<
                                                                      1,0,1,0,0,0,
                    0,1,0,1,0,0,
                    0,0,1,0,0,0,
                    0,0,0,1,0,0,
                    0,0,0,0,1,0,
                    0,0,0,0,0,1);

            tracked_faces.push_back(new_track);
        }
    }

    // Remove lost tracks
    tracked_faces.erase(
            std::remove_if(tracked_faces.begin(), tracked_faces.end(),
                           [](const TrackedFace& t) { return t.missing_frames > 30; }),
            tracked_faces.end());
}

float RetinafaceDetector::calculateIOU(const cv::Rect& box1, const cv::Rect& box2) {
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);

    if (x1 >= x2 || y1 >= y2)
        return 0.0f;

    float intersection_area = (x2 - x1) * (y2 - y1);
    float box1_area = box1.width * box1.height;
    float box2_area = box2.width * box2.height;

    return intersection_area / (box1_area + box2_area - intersection_area);
}
void RetinafaceDetector::initializeAnchors() {
    // Initialize feature map sizes for 640x640 input
    std::vector<int> feature_maps = {80, 40, 20, 10, 5};  // P2, P3, P4, P5, P6

    // Initialize anchor sizes
    std::vector<int> anchor_sizes = {16, 32, 64, 128, 256};

    // Initialize anchor ratios
    std::vector<float> ratios = {1.0f};

    // Generate base anchors
    anchor_base.clear();
    anchor_scales = {1.0f, 1.25f, 1.5f};

    for (size_t k = 0; k < feature_maps.size(); k++) {
        int size = anchor_sizes[k];
        for (float ratio : ratios) {
            float w = size * std::sqrt(ratio);
            float h = size / std::sqrt(ratio);

            for (float scale : anchor_scales) {
                anchor_base.push_back({w * scale, h * scale});
            }
        }
    }

    std::cout << "Initialized " << anchor_base.size() << " base anchors" << std::endl;
}