#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "YOLODetector.h"

namespace FootballAnalytics {

/**
 * @brief 帧数据结构
 * 
 * 包含一帧的所有检测和分析结果
 */
struct FrameData {
    int frameNumber;
    int64_t timestamp;
    std::string videoSource;
    
    // 检测数据
    std::vector<Detection> players;
    std::vector<Detection> keypoints;
    std::vector<Detection> balls;
    
    // 分析数据
    std::vector<cv::Point2f> tacMapPositions;  // 战术地图坐标
    std::vector<int> teamIds;                   // 球队ID
    
    FrameData() : frameNumber(0), timestamp(0) {}
};

/**
 * @brief HTTP API客户端类
 * 
 * 负责将检测结果通过HTTP API发送到服务器
 */
class ApiClient {
public:
    /**
     * @brief 构造函数
     * @param baseUrl API服务器基础URL（例如：http://localhost:8080）
     * @param apiKey API密钥（可选）
     */
    explicit ApiClient(const std::string& baseUrl, const std::string& apiKey = "");
    
    /**
     * @brief 析构函数
     */
    ~ApiClient();
    
    /**
     * @brief 发送帧数据到服务器
     * @param data 帧数据
     * @return 成功返回true
     */
    bool sendFrameData(const FrameData& data);
    
    /**
     * @brief 批量发送帧数据
     * @param dataList 帧数据列表
     * @return 成功返回true
     */
    bool sendBatchFrameData(const std::vector<FrameData>& dataList);
    
    /**
     * @brief 发送视频处理开始通知
     * @param videoSource 视频源标识
     * @param totalFrames 总帧数
     * @return 成功返回true
     */
    bool notifyVideoStart(const std::string& videoSource, int totalFrames);
    
    /**
     * @brief 发送视频处理完成通知
     * @param videoSource 视频源标识
     * @return 成功返回true
     */
    bool notifyVideoComplete(const std::string& videoSource);
    
    /**
     * @brief 测试连接
     * @return 连接成功返回true
     */
    bool testConnection();
    
    /**
     * @brief 设置超时时间（秒）
     */
    void setTimeout(int seconds) { timeoutSeconds_ = seconds; }
    
    /**
     * @brief 启用/禁用调试输出
     */
    void setDebug(bool enable) { debugMode_ = enable; }

private:
    std::string baseUrl_;
    std::string apiKey_;
    int timeoutSeconds_;
    bool debugMode_;
    
    /**
     * @brief 将帧数据转换为JSON字符串
     */
    std::string frameDataToJson(const FrameData& data);
    
    /**
     * @brief 发送POST请求
     * @param endpoint API端点路径
     * @param jsonData JSON数据
     * @return 成功返回true
     */
    bool sendPostRequest(const std::string& endpoint, const std::string& jsonData);
    
    /**
     * @brief 提取主机名和端口
     */
    void parseBaseUrl();
    
    std::string host_;
    int port_;
    std::string basePath_;
};

} // namespace FootballAnalytics
