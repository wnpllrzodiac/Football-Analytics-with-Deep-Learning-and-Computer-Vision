#pragma once

#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "YOLODetector.h"

namespace FootballAnalytics {

/**
 * @brief 坐标转换类
 * 
 * 负责球场关键点检测和坐标系转换（单应性变换）
 */
class CoordinateTransform {
public:
    /**
     * @brief 构造函数
     * @param keypointMapFile 球场关键点映射配置文件（JSON）
     * @param displacementTolerance 关键点位移容差（像素）
     */
    explicit CoordinateTransform(const std::string& keypointMapFile,
                                 float displacementTolerance = 7.0f);
    
    /**
     * @brief 析构函数
     */
    ~CoordinateTransform();
    
    /**
     * @brief 加载战术地图
     * @param tacticalMapPath 战术地图图像路径
     * @return 成功返回true
     */
    bool loadTacticalMap(const std::string& tacticalMapPath);
    
    /**
     * @brief 计算单应性矩阵
     * @param keypointDetections 检测到的关键点列表
     * @param frameNumber 当前帧号
     * @return 是否成功计算/更新单应性矩阵
     */
    bool computeHomography(const std::vector<Detection>& keypointDetections,
                          int frameNumber);
    
    /**
     * @brief 将球员坐标转换到战术地图
     * @param playerDetections 球员检测结果
     * @return 战术地图上的坐标列表
     */
    std::vector<cv::Point2f> transformToTacticalMap(
        const std::vector<Detection>& playerDetections);
    
    /**
     * @brief 将单个点转换到战术地图
     * @param point 原始图像上的点
     * @return 战术地图上的点
     */
    cv::Point2f transformPoint(const cv::Point2f& point);
    
    /**
     * @brief 获取当前单应性矩阵
     */
    cv::Mat getHomography() const { return homography_; }
    
    /**
     * @brief 检查单应性矩阵是否有效
     */
    bool hasValidHomography() const { return !homography_.empty(); }
    
    /**
     * @brief 设置类别标签映射
     * @param classLabels 关键点类别标签列表
     */
    void setKeypointClassLabels(const std::vector<std::string>& classLabels);

private:
    cv::Mat homography_;                              // 当前单应性矩阵
    cv::Mat tacticalMap_;                             // 战术地图图像
    
    std::map<std::string, cv::Point2f> keypointMap_;  // 关键点名称到战术地图坐标的映射
    std::vector<std::string> keypointLabels_;         // 关键点标签列表
    
    std::vector<cv::Point2f> prevKeypointsSrc_;       // 上一帧的关键点源坐标
    std::vector<std::string> prevKeypointLabels_;     // 上一帧的关键点标签
    
    float displacementTolerance_;                     // 关键点位移容差
    int lastUpdateFrame_;                             // 上次更新单应性矩阵的帧号
    
    /**
     * @brief 加载关键点映射配置
     */
    bool loadKeypointMap(const std::string& jsonFile);
    
    /**
     * @brief 计算关键点位移的RMSE
     */
    float computeKeypointDisplacementRMSE(
        const std::vector<cv::Point2f>& prev,
        const std::vector<cv::Point2f>& curr);
};

} // namespace FootballAnalytics
