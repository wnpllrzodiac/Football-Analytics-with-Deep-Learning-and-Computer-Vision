#include "YOLODetector.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace FootballAnalytics {

YOLODetector::YOLODetector(const std::string& modelPath,
                           float confThreshold,
                           float iouThreshold)
    : inputSize_(640, 640)
    , confThreshold_(confThreshold)
    , iouThreshold_(iouThreshold)
{
    try {
        // 创建ONNX Runtime环境
        env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FootballAnalytics");
        
        // 创建会话选项
        sessionOptions_ = std::make_unique<Ort::SessionOptions>();
        sessionOptions_->SetIntraOpNumThreads(4);
        sessionOptions_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        
        // 尝试使用CUDA（如果可用）
        // Uncomment these lines if you have CUDA support
        // OrtCUDAProviderOptions cuda_options;
        // sessionOptions_->AppendExecutionProvider_CUDA(cuda_options);
        
        // 创建会话
#ifdef _WIN32
        std::wstring wideModelPath(modelPath.begin(), modelPath.end());
        session_ = std::make_unique<Ort::Session>(*env_, wideModelPath.c_str(), *sessionOptions_);
#else
        session_ = std::make_unique<Ort::Session>(*env_, modelPath.c_str(), *sessionOptions_);
#endif
        
        // 获取输入输出信息
        Ort::AllocatorWithDefaultOptions allocator;
        
        // 输入节点
        size_t numInputNodes = session_->GetInputCount();
        for (size_t i = 0; i < numInputNodes; i++) {
            auto inputName = session_->GetInputNameAllocated(i, allocator);
            inputNamesStr_.push_back(inputName.get());
            inputNames_.push_back(inputNamesStr_.back().c_str());
        }
        
        // 输出节点
        size_t numOutputNodes = session_->GetOutputCount();
        for (size_t i = 0; i < numOutputNodes; i++) {
            auto outputName = session_->GetOutputNameAllocated(i, allocator);
            outputNamesStr_.push_back(outputName.get());
            outputNames_.push_back(outputNamesStr_.back().c_str());
        }
        
        // 获取输入尺寸
        auto inputTypeInfo = session_->GetInputTypeInfo(0);
        auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
        auto inputDims = inputTensorInfo.GetShape();
        
        if (inputDims.size() >= 4) {
            // 通常是 [batch, channels, height, width]
            inputSize_.height = static_cast<int>(inputDims[2]);
            inputSize_.width = static_cast<int>(inputDims[3]);
        }
        
        std::cout << "YOLO Detector initialized successfully" << std::endl;
        std::cout << "  Model: " << modelPath << std::endl;
        std::cout << "  Input size: " << inputSize_.width << "x" << inputSize_.height << std::endl;
        std::cout << "  Confidence threshold: " << confThreshold_ << std::endl;
        std::cout << "  IoU threshold: " << iouThreshold_ << std::endl;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
        throw std::runtime_error("Failed to initialize YOLODetector: " + std::string(e.what()));
    }
}

YOLODetector::~YOLODetector() {
    // 智能指针会自动清理
}

void YOLODetector::setClassLabels(const std::vector<std::string>& labels) {
    classLabels_ = labels;
}

std::vector<float> YOLODetector::preprocess(const cv::Mat& frame) {
    cv::Mat resized;
    cv::resize(frame, resized, inputSize_);
    
    // 转换为RGB
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    // 归一化到[0, 1]
    rgb.convertTo(rgb, CV_32F, 1.0 / 255.0);
    
    // 转换为CHW格式（NCHW: batch, channel, height, width）
    std::vector<float> inputTensor;
    inputTensor.resize(3 * inputSize_.height * inputSize_.width);
    
    std::vector<cv::Mat> channels(3);
    cv::split(rgb, channels);
    
    int channelSize = inputSize_.height * inputSize_.width;
    for (int c = 0; c < 3; c++) {
        std::memcpy(inputTensor.data() + c * channelSize,
                   channels[c].data,
                   channelSize * sizeof(float));
    }
    
    return inputTensor;
}

std::vector<Detection> YOLODetector::detect(const cv::Mat& frame) {
    if (frame.empty()) {
        return {};
    }
    
    cv::Size originalSize = frame.size();
    
    // 预处理
    std::vector<float> inputTensor = preprocess(frame);
    
    // 创建输入tensor
    std::vector<int64_t> inputShape = {1, 3, inputSize_.height, inputSize_.width};
    
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    
    Ort::Value inputTensorValue = Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensor.data(), inputTensor.size(),
        inputShape.data(), inputShape.size());
    
    // 运行推理
    auto outputTensors = session_->Run(
        Ort::RunOptions{nullptr},
        inputNames_.data(), &inputTensorValue, 1,
        outputNames_.data(), outputNames_.size());
    
    // 获取输出
    float* outputData = outputTensors[0].GetTensorMutableData<float>();
    auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
    
    // 计算输出大小
    size_t outputSize = 1;
    for (auto dim : outputShape) {
        outputSize *= dim;
    }
    
    std::vector<float> output(outputData, outputData + outputSize);
    
    // 后处理
    std::vector<Detection> detections = postprocess(output, originalSize);
    
    // NMS
    detections = nms(detections);
    
    return detections;
}

std::vector<Detection> YOLODetector::postprocess(const std::vector<float>& output,
                                                 const cv::Size& frameSize) {
    std::vector<Detection> detections;
    
    // YOLOv8输出格式：[batch, 4+num_classes, num_anchors]
    // 需要转置为 [num_anchors, 4+num_classes]
    
    // 假设输出形状为 [1, 84, 8400] for 80 classes
    // 或 [1, num_classes+4, num_anchors]
    
    int numClasses = 80; // 默认值，可根据实际调整
    int numAnchors = 8400; // 640x640输入的默认值
    
    // 动态推断类别数
    if (output.size() > 0) {
        numAnchors = static_cast<int>(output.size() / (4 + numClasses));
        if (!classLabels_.empty()) {
            numClasses = static_cast<int>(classLabels_.size());
        }
    }
    
    float scaleX = static_cast<float>(frameSize.width) / inputSize_.width;
    float scaleY = static_cast<float>(frameSize.height) / inputSize_.height;
    
    // 解析检测结果
    for (int i = 0; i < numAnchors; i++) {
        // 获取边界框信息
        float cx = output[i];
        float cy = output[numAnchors + i];
        float w = output[2 * numAnchors + i];
        float h = output[3 * numAnchors + i];
        
        // 查找最高置信度的类别
        float maxConf = 0.0f;
        int maxClassId = -1;
        
        for (int c = 0; c < numClasses; c++) {
            float conf = output[(4 + c) * numAnchors + i];
            if (conf > maxConf) {
                maxConf = conf;
                maxClassId = c;
            }
        }
        
        // 过滤低置信度检测
        if (maxConf > confThreshold_) {
            Detection det;
            
            // 转换到原始图像坐标
            det.bbox.x = static_cast<int>((cx - w / 2.0f) * scaleX);
            det.bbox.y = static_cast<int>((cy - h / 2.0f) * scaleY);
            det.bbox.width = static_cast<int>(w * scaleX);
            det.bbox.height = static_cast<int>(h * scaleY);
            
            det.classId = maxClassId;
            det.confidence = maxConf;
            det.center = cv::Point2f(cx * scaleX, cy * scaleY);
            
            // 设置标签
            if (maxClassId < static_cast<int>(classLabels_.size())) {
                det.label = classLabels_[maxClassId];
            } else {
                det.label = "class_" + std::to_string(maxClassId);
            }
            
            detections.push_back(det);
        }
    }
    
    return detections;
}

float YOLODetector::computeIoU(const cv::Rect& box1, const cv::Rect& box2) {
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);
    
    int intersectionWidth = std::max(0, x2 - x1);
    int intersectionHeight = std::max(0, y2 - y1);
    int intersectionArea = intersectionWidth * intersectionHeight;
    
    int box1Area = box1.width * box1.height;
    int box2Area = box2.width * box2.height;
    int unionArea = box1Area + box2Area - intersectionArea;
    
    return static_cast<float>(intersectionArea) / unionArea;
}

std::vector<Detection> YOLODetector::nms(const std::vector<Detection>& detections) {
    std::vector<Detection> result;
    
    // 按置信度排序
    std::vector<Detection> sorted = detections;
    std::sort(sorted.begin(), sorted.end(),
             [](const Detection& a, const Detection& b) {
                 return a.confidence > b.confidence;
             });
    
    std::vector<bool> suppressed(sorted.size(), false);
    
    for (size_t i = 0; i < sorted.size(); i++) {
        if (suppressed[i]) continue;
        
        result.push_back(sorted[i]);
        
        // 抑制重叠的检测
        for (size_t j = i + 1; j < sorted.size(); j++) {
            if (suppressed[j]) continue;
            
            // 只对相同类别进行NMS
            if (sorted[i].classId == sorted[j].classId) {
                float iou = computeIoU(sorted[i].bbox, sorted[j].bbox);
                if (iou > iouThreshold_) {
                    suppressed[j] = true;
                }
            }
        }
    }
    
    return result;
}

} // namespace FootballAnalytics
