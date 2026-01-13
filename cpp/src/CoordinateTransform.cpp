#include "CoordinateTransform.h"
#include <iostream>
#include <fstream>
#include <cmath>

// 简单的JSON解析（实际项目中应使用nlohmann/json或类似库）
// 这里提供一个简化版本
namespace FootballAnalytics {

CoordinateTransform::CoordinateTransform(const std::string& keypointMapFile,
                                         float displacementTolerance)
    : displacementTolerance_(displacementTolerance)
    , lastUpdateFrame_(-1)
{
    if (!loadKeypointMap(keypointMapFile)) {
        std::cerr << "Warning: Failed to load keypoint map from " << keypointMapFile << std::endl;
    }
}

CoordinateTransform::~CoordinateTransform() {
}

bool CoordinateTransform::loadTacticalMap(const std::string& tacticalMapPath) {
    tacticalMap_ = cv::imread(tacticalMapPath);
    
    if (tacticalMap_.empty()) {
        std::cerr << "Failed to load tactical map: " << tacticalMapPath << std::endl;
        return false;
    }
    
    std::cout << "Tactical map loaded: " << tacticalMapPath << std::endl;
    std::cout << "  Size: " << tacticalMap_.cols << "x" << tacticalMap_.rows << std::endl;
    
    return true;
}

bool CoordinateTransform::loadKeypointMap(const std::string& jsonFile) {
    std::ifstream file(jsonFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open keypoint map file: " << jsonFile << std::endl;
        return false;
    }
    
    // 简化的JSON解析
    // 实际项目中应使用 nlohmann/json 库
    // 格式示例：
    // {
    //   "Center circle": [640, 360],
    //   "Left penalty spot": [150, 360],
    //   ...
    // }
    
    std::string line;
    std::string currentKey;
    
    while (std::getline(file, line)) {
        // 移除空格
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (line.empty() || line[0] == '{' || line[0] == '}') continue;
        
        // 查找键值对
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            // 提取键（去除引号和逗号）
            std::string key = line.substr(0, colonPos);
            key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
            key.erase(std::remove(key.begin(), key.end(), ','), key.end());
            
            // 提取值（数组）
            std::string value = line.substr(colonPos + 1);
            size_t leftBracket = value.find('[');
            size_t rightBracket = value.find(']');
            
            if (leftBracket != std::string::npos && rightBracket != std::string::npos) {
                std::string coords = value.substr(leftBracket + 1, rightBracket - leftBracket - 1);
                size_t commaPos = coords.find(',');
                
                if (commaPos != std::string::npos) {
                    float x = std::stof(coords.substr(0, commaPos));
                    float y = std::stof(coords.substr(commaPos + 1));
                    
                    keypointMap_[key] = cv::Point2f(x, y);
                }
            }
        }
    }
    
    file.close();
    
    std::cout << "Keypoint map loaded: " << keypointMap_.size() << " points" << std::endl;
    
    return !keypointMap_.empty();
}

void CoordinateTransform::setKeypointClassLabels(const std::vector<std::string>& classLabels) {
    keypointLabels_ = classLabels;
}

bool CoordinateTransform::computeHomography(const std::vector<Detection>& keypointDetections,
                                           int frameNumber) {
    if (keypointDetections.size() < 4) {
        // 需要至少4个点来计算单应性矩阵
        return false;
    }
    
    // 提取检测到的关键点坐标和标签
    std::vector<cv::Point2f> srcPoints;
    std::vector<cv::Point2f> dstPoints;
    std::vector<std::string> currentLabels;
    
    for (const auto& det : keypointDetections) {
        // 检查标签是否在映射中
        auto it = keypointMap_.find(det.label);
        if (it != keypointMap_.end()) {
            srcPoints.push_back(det.center);
            dstPoints.push_back(it->second);
            currentLabels.push_back(det.label);
        }
    }
    
    if (srcPoints.size() < 4) {
        return false;
    }
    
    // 判断是否需要更新单应性矩阵
    bool shouldUpdate = false;
    
    if (frameNumber <= 0 || lastUpdateFrame_ < 0) {
        // 第一帧，必须更新
        shouldUpdate = true;
    } else if (prevKeypointsSrc_.size() >= 4 && currentLabels.size() >= 4) {
        // 查找共同的关键点
        std::vector<cv::Point2f> commonPrev;
        std::vector<cv::Point2f> commonCurr;
        
        for (size_t i = 0; i < currentLabels.size(); i++) {
            for (size_t j = 0; j < prevKeypointLabels_.size(); j++) {
                if (currentLabels[i] == prevKeypointLabels_[j]) {
                    commonCurr.push_back(srcPoints[i]);
                    commonPrev.push_back(prevKeypointsSrc_[j]);
                    break;
                }
            }
        }
        
        if (commonPrev.size() >= 4) {
            // 计算关键点位移
            float rmse = computeKeypointDisplacementRMSE(commonPrev, commonCurr);
            
            if (rmse > displacementTolerance_) {
                shouldUpdate = true;
            }
        } else {
            shouldUpdate = true;
        }
    } else {
        shouldUpdate = true;
    }
    
    if (shouldUpdate) {
        // 计算单应性矩阵
        cv::Mat H = cv::findHomography(srcPoints, dstPoints, cv::RANSAC, 3.0);
        
        if (!H.empty()) {
            homography_ = H;
            prevKeypointsSrc_ = srcPoints;
            prevKeypointLabels_ = currentLabels;
            lastUpdateFrame_ = frameNumber;
            
            std::cout << "Homography updated at frame " << frameNumber 
                     << " with " << srcPoints.size() << " keypoints" << std::endl;
            return true;
        }
    }
    
    return !homography_.empty();
}

float CoordinateTransform::computeKeypointDisplacementRMSE(
    const std::vector<cv::Point2f>& prev,
    const std::vector<cv::Point2f>& curr) {
    
    if (prev.size() != curr.size() || prev.empty()) {
        return std::numeric_limits<float>::max();
    }
    
    float sumSquaredError = 0.0f;
    
    for (size_t i = 0; i < prev.size(); i++) {
        float dx = curr[i].x - prev[i].x;
        float dy = curr[i].y - prev[i].y;
        sumSquaredError += (dx * dx + dy * dy);
    }
    
    return std::sqrt(sumSquaredError / prev.size());
}

std::vector<cv::Point2f> CoordinateTransform::transformToTacticalMap(
    const std::vector<Detection>& playerDetections) {
    
    std::vector<cv::Point2f> tacMapPositions;
    
    if (homography_.empty()) {
        return tacMapPositions;
    }
    
    for (const auto& det : playerDetections) {
        // 使用球员边界框底部中心点（脚的位置）
        cv::Point2f footPos(det.bbox.x + det.bbox.width / 2.0f,
                           det.bbox.y + det.bbox.height);
        
        cv::Point2f transformed = transformPoint(footPos);
        tacMapPositions.push_back(transformed);
    }
    
    return tacMapPositions;
}

cv::Point2f CoordinateTransform::transformPoint(const cv::Point2f& point) {
    if (homography_.empty()) {
        return point;
    }
    
    // 转换为齐次坐标
    cv::Mat src = (cv::Mat_<double>(3, 1) << point.x, point.y, 1.0);
    
    // 应用单应性变换
    cv::Mat dst = homography_ * src;
    
    // 转换回笛卡尔坐标
    double w = dst.at<double>(2, 0);
    if (std::abs(w) < 1e-6) {
        return point; // 避免除以零
    }
    
    return cv::Point2f(
        static_cast<float>(dst.at<double>(0, 0) / w),
        static_cast<float>(dst.at<double>(1, 0) / w)
    );
}

} // namespace FootballAnalytics
