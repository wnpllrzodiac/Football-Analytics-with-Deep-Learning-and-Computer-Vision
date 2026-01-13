#pragma once

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

namespace FootballAnalytics {

/**
 * @brief 检测结果结构体
 */
struct Detection {
    cv::Rect bbox;          // 边界框 (x, y, width, height)
    int classId;            // 类别ID
    float confidence;       // 置信度
    cv::Point2f center;     // 中心点坐标
    std::string label;      // 类别标签
    
    Detection() : classId(-1), confidence(0.0f) {}
};

/**
 * @brief YOLO检测器类
 * 
 * 使用ONNX Runtime进行YOLOv8模型推理
 */
class YOLODetector {
public:
    /**
     * @brief 构造函数
     * @param modelPath ONNX模型路径
     * @param confThreshold 置信度阈值（默认0.6）
     * @param iouThreshold NMS的IoU阈值（默认0.45）
     */
    explicit YOLODetector(const std::string& modelPath, 
                         float confThreshold = 0.6f,
                         float iouThreshold = 0.45f);
    
    /**
     * @brief 析构函数
     */
    ~YOLODetector();
    
    // 禁止拷贝
    YOLODetector(const YOLODetector&) = delete;
    YOLODetector& operator=(const YOLODetector&) = delete;
    
    /**
     * @brief 检测图像中的目标
     * @param frame 输入图像
     * @return 检测结果列表
     */
    std::vector<Detection> detect(const cv::Mat& frame);
    
    /**
     * @brief 设置类别标签
     * @param labels 类别标签列表
     */
    void setClassLabels(const std::vector<std::string>& labels);
    
    /**
     * @brief 设置置信度阈值
     */
    void setConfThreshold(float threshold) { confThreshold_ = threshold; }
    
    /**
     * @brief 设置IoU阈值
     */
    void setIouThreshold(float threshold) { iouThreshold_ = threshold; }
    
    /**
     * @brief 获取输入尺寸
     */
    cv::Size getInputSize() const { return inputSize_; }

private:
    std::unique_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::Session> session_;
    std::unique_ptr<Ort::SessionOptions> sessionOptions_;
    
    std::vector<const char*> inputNames_;
    std::vector<const char*> outputNames_;
    std::vector<std::string> inputNamesStr_;
    std::vector<std::string> outputNamesStr_;
    
    cv::Size inputSize_;
    float confThreshold_;
    float iouThreshold_;
    
    std::vector<std::string> classLabels_;
    
    /**
     * @brief 预处理图像
     * @param frame 原始图像
     * @return 预处理后的tensor数据
     */
    std::vector<float> preprocess(const cv::Mat& frame);
    
    /**
     * @brief 后处理检测结果
     * @param output 模型输出
     * @param frameSize 原始图像尺寸
     * @return 检测结果列表
     */
    std::vector<Detection> postprocess(const std::vector<float>& output,
                                      const cv::Size& frameSize);
    
    /**
     * @brief 非极大值抑制（NMS）
     * @param detections 输入检测结果
     * @return 过滤后的检测结果
     */
    std::vector<Detection> nms(const std::vector<Detection>& detections);
    
    /**
     * @brief 计算两个框的IoU
     */
    float computeIoU(const cv::Rect& box1, const cv::Rect& box2);
};

} // namespace FootballAnalytics
