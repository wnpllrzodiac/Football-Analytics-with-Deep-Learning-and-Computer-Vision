#pragma once

#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "YOLODetector.h"

namespace FootballAnalytics {

/**
 * @brief 球队颜色信息
 */
struct TeamColorInfo {
    std::string teamName;
    cv::Scalar playerColor;    // RGB格式
    cv::Scalar goalkeeperColor; // RGB格式
    
    TeamColorInfo() {}
    TeamColorInfo(const std::string& name, 
                 const cv::Scalar& pColor,
                 const cv::Scalar& gkColor)
        : teamName(name), playerColor(pColor), goalkeeperColor(gkColor) {}
};

/**
 * @brief 球队预测类
 * 
 * 基于颜色分析预测球员所属球队
 */
class TeamPredictor {
public:
    /**
     * @brief 构造函数
     * @param numPaletteColors 提取的调色板颜色数量（默认3）
     */
    explicit TeamPredictor(int numPaletteColors = 3);
    
    /**
     * @brief 析构函数
     */
    ~TeamPredictor();
    
    /**
     * @brief 设置球队颜色信息
     * @param team1 第一支球队颜色信息
     * @param team2 第二支球队颜色信息
     */
    void setTeamColors(const TeamColorInfo& team1, const TeamColorInfo& team2);
    
    /**
     * @brief 预测球员所属球队
     * @param frame 原始图像
     * @param playerDetections 球员检测结果
     * @return 球队ID列表（0=team1, 1=team2）
     */
    std::vector<int> predictTeams(const cv::Mat& frame,
                                  const std::vector<Detection>& playerDetections);
    
    /**
     * @brief 获取球员的主要颜色调色板
     * @param frame 原始图像
     * @param bbox 球员边界框
     * @return 主要颜色列表（RGB格式）
     */
    std::vector<cv::Scalar> extractColorPalette(const cv::Mat& frame,
                                               const cv::Rect& bbox);
    
    /**
     * @brief 获取球队数量
     */
    int getNumTeams() const { return static_cast<int>(teamColors_.size()); }
    
    /**
     * @brief 获取球队信息
     */
    const std::vector<TeamColorInfo>& getTeamColors() const { return teamColors_; }

private:
    int numPaletteColors_;
    std::vector<TeamColorInfo> teamColors_;
    std::vector<cv::Scalar> referenceColors_; // 所有参考颜色（LAB空间）
    
    /**
     * @brief RGB转LAB色彩空间
     */
    cv::Scalar rgbToLab(const cv::Scalar& rgb);
    
    /**
     * @brief 计算LAB空间的欧几里得距离
     */
    float computeLabDistance(const cv::Scalar& lab1, const cv::Scalar& lab2);
    
    /**
     * @brief 根据颜色距离预测球队
     * @param palette 球员的颜色调色板（LAB空间）
     * @return 球队ID
     */
    int predictTeamFromPalette(const std::vector<cv::Scalar>& palette);
};

} // namespace FootballAnalytics
