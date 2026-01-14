#include "TeamPredictor.h"
#include <iostream>
#include <algorithm>
#include <map>

namespace FootballAnalytics {

TeamPredictor::TeamPredictor(int numPaletteColors)
    : numPaletteColors_(numPaletteColors)
{
}

TeamPredictor::~TeamPredictor() {
}

void TeamPredictor::setTeamColors(const TeamColorInfo& team1, const TeamColorInfo& team2) {
    teamColors_.clear();
    referenceColors_.clear();
    
    teamColors_.push_back(team1);
    teamColors_.push_back(team2);
    
    // 转换所有参考颜色到LAB空间
    referenceColors_.push_back(rgbToLab(team1.playerColor));
    referenceColors_.push_back(rgbToLab(team1.goalkeeperColor));
    referenceColors_.push_back(rgbToLab(team2.playerColor));
    referenceColors_.push_back(rgbToLab(team2.goalkeeperColor));
    
    std::cout << "Team colors configured:" << std::endl;
    std::cout << "  Team 1: " << team1.teamName << std::endl;
    std::cout << "  Team 2: " << team2.teamName << std::endl;
}

cv::Scalar TeamPredictor::rgbToLab(const cv::Scalar& rgb) {
    // 创建单像素图像
    cv::Mat rgbMat(1, 1, CV_8UC3, rgb);
    cv::Mat labMat;
    
    // 转换到LAB空间
    cv::cvtColor(rgbMat, labMat, cv::COLOR_RGB2Lab);
    
    return labMat.at<cv::Vec3b>(0, 0);
}

float TeamPredictor::computeLabDistance(const cv::Scalar& lab1, const cv::Scalar& lab2) {
    // CIE76颜色差异公式
    float dL = lab1[0] - lab2[0];
    float da = lab1[1] - lab2[1];
    float db = lab1[2] - lab2[2];
    
    return std::sqrt(dL * dL + da * da + db * db);
}

std::vector<cv::Scalar> TeamPredictor::extractColorPalette(const cv::Mat& frame,
                                                          const cv::Rect& bbox) {
    std::vector<cv::Scalar> palette;
    
    // 确保边界框在图像范围内
    cv::Rect safeBbox = bbox & cv::Rect(0, 0, frame.cols, frame.rows);
    if (safeBbox.width <= 0 || safeBbox.height <= 0) {
        return palette;
    }
    
    // 裁剪球员图像
    cv::Mat playerImg = frame(safeBbox).clone();
    
    // 转换到RGB
    cv::Mat playerRgb;
    cv::cvtColor(playerImg, playerRgb, cv::COLOR_BGR2RGB);
    
    // 提取中心区域（球衣区域）
    int centerX1 = std::max(playerRgb.cols / 2 - playerRgb.cols / 5, 0);
    int centerX2 = std::min(playerRgb.cols / 2 + playerRgb.cols / 5, playerRgb.cols);
    int centerY1 = std::max(playerRgb.rows / 3 - playerRgb.rows / 5, 0);
    int centerY2 = std::min(playerRgb.rows / 3 + playerRgb.rows / 5, playerRgb.rows);
    
    cv::Rect centerRegion(centerX1, centerY1, centerX2 - centerX1, centerY2 - centerY1);
    
    // 验证中心区域有效性
    if (centerRegion.width <= 0 || centerRegion.height <= 0) {
        return palette;
    }
    
    cv::Mat centerImg = playerRgb(centerRegion);
    
    // 检查提取的图像是否为空
    if (centerImg.empty() || centerImg.rows == 0 || centerImg.cols == 0) {
        return palette;
    }
    
    // 转换为浮点数
    cv::Mat floatImg;
    centerImg.convertTo(floatImg, CV_32F);
    
    // 重塑为一维数组
    cv::Mat samples = floatImg.reshape(1, floatImg.rows * floatImg.cols);
    
    // 验证 samples 的维度和类型
    if (samples.empty() || samples.dims > 2 || samples.type() != CV_32FC3) {
        return palette;
    }
    
    // 检查样本数量是否足够进行聚类
    int numSamples = samples.rows;
    if (numSamples < numPaletteColors_) {
        // 样本数不足，直接计算平均颜色
        if (numSamples > 0) {
            cv::Scalar meanColor = cv::mean(centerImg);
            palette.push_back(meanColor);
        }
        return palette;
    }
    
    // 使用K-means聚类提取主要颜色
    int clusterCount = numPaletteColors_;
    cv::Mat labels, centers;
    
    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 0.2);
    cv::kmeans(samples, clusterCount, labels, criteria, 3, cv::KMEANS_PP_CENTERS, centers);
    
    // 统计每个簇的像素数
    std::vector<int> counts(clusterCount, 0);
    for (int i = 0; i < labels.rows; i++) {
        counts[labels.at<int>(i)]++;
    }
    
    // 按像素数排序
    std::vector<std::pair<int, int>> sortedClusters; // (count, index)
    for (int i = 0; i < clusterCount; i++) {
        sortedClusters.push_back({counts[i], i});
    }
    std::sort(sortedClusters.begin(), sortedClusters.end(),
             [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // 提取排序后的颜色
    for (int i = 0; i < clusterCount; i++) {
        int idx = sortedClusters[i].second;
        cv::Scalar color(
            centers.at<float>(idx, 0),
            centers.at<float>(idx, 1),
            centers.at<float>(idx, 2)
        );
        palette.push_back(color);
    }
    
    return palette;
}

int TeamPredictor::predictTeamFromPalette(const std::vector<cv::Scalar>& palette) {
    if (palette.empty() || referenceColors_.empty()) {
        return 0; // 默认返回第一支球队
    }
    
    // 为每种颜色投票
    std::vector<int> votes;
    
    for (const auto& color : palette) {
        // 计算与所有参考颜色的距离
        float minDistance = std::numeric_limits<float>::max();
        int closestTeam = 0;
        
        for (size_t i = 0; i < referenceColors_.size(); i++) {
            float distance = computeLabDistance(color, referenceColors_[i]);
            
            if (distance < minDistance) {
                minDistance = distance;
                // 每支球队有2种颜色（球员+守门员）
                closestTeam = i / 2;
            }
        }
        
        votes.push_back(closestTeam);
    }
    
    // 统计投票
    std::map<int, int> voteCounts;
    for (int vote : votes) {
        voteCounts[vote]++;
    }
    
    // 返回得票最多的球队
    int predictedTeam = 0;
    int maxVotes = 0;
    
    for (const auto& pair : voteCounts) {
        if (pair.second > maxVotes) {
            maxVotes = pair.second;
            predictedTeam = pair.first;
        }
    }
    
    return predictedTeam;
}

std::vector<int> TeamPredictor::predictTeams(const cv::Mat& frame,
                                            const std::vector<Detection>& playerDetections) {
    std::vector<int> teamIds;
    
    if (teamColors_.empty()) {
        std::cerr << "Warning: Team colors not configured" << std::endl;
        return teamIds;
    }
    
    // 转换图像到RGB
    cv::Mat frameRgb;
    cv::cvtColor(frame, frameRgb, cv::COLOR_BGR2RGB);
    
    for (const auto& det : playerDetections) {
        // 只处理球员（classId == 0）
        if (det.classId == 0) {
            // 提取颜色调色板（RGB空间）
            std::vector<cv::Scalar> paletteRgb = extractColorPalette(frameRgb, det.bbox);
            
            // 转换到LAB空间
            std::vector<cv::Scalar> paletteLab;
            for (const auto& color : paletteRgb) {
                paletteLab.push_back(rgbToLab(color));
            }
            
            // 预测球队
            int teamId = predictTeamFromPalette(paletteLab);
            teamIds.push_back(teamId);
        }
    }
    
    return teamIds;
}

} // namespace FootballAnalytics
