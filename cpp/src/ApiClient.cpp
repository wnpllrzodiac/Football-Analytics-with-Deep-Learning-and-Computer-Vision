#include "ApiClient.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

// 使用cpp-httplib库（header-only）
// 下载地址：https://github.com/yhirose/cpp-httplib
#include "httplib.h"

namespace FootballAnalytics {

ApiClient::ApiClient(const std::string& baseUrl, const std::string& apiKey)
    : baseUrl_(baseUrl)
    , apiKey_(apiKey)
    , timeoutSeconds_(30)
    , debugMode_(false)
    , port_(80)
{
    parseBaseUrl();
    
    std::cout << "API Client initialized:" << std::endl;
    std::cout << "  Base URL: " << baseUrl_ << std::endl;
    std::cout << "  Host: " << host_ << std::endl;
    std::cout << "  Port: " << port_ << std::endl;
}

ApiClient::~ApiClient() {
}

void ApiClient::parseBaseUrl() {
    // 解析URL：http://host:port/path
    std::string url = baseUrl_;
    
    // 移除协议
    size_t protoPos = url.find("://");
    if (protoPos != std::string::npos) {
        url = url.substr(protoPos + 3);
    }
    
    // 查找路径分隔符
    size_t pathPos = url.find('/');
    std::string hostPort = (pathPos != std::string::npos) ? url.substr(0, pathPos) : url;
    basePath_ = (pathPos != std::string::npos) ? url.substr(pathPos) : "";
    
    // 分离主机和端口
    size_t colonPos = hostPort.find(':');
    if (colonPos != std::string::npos) {
        host_ = hostPort.substr(0, colonPos);
        port_ = std::stoi(hostPort.substr(colonPos + 1));
    } else {
        host_ = hostPort;
        // 根据协议设置默认端口
        if (baseUrl_.find("https://") == 0) {
            port_ = 443;
        } else {
            port_ = 80;
        }
    }
}

std::string ApiClient::frameDataToJson(const FrameData& data) {
    std::ostringstream json;
    
    json << "{";
    json << "\"frameNumber\":" << data.frameNumber << ",";
    json << "\"timestamp\":" << data.timestamp << ",";
    json << "\"videoSource\":\"" << data.videoSource << "\",";
    
    // 球员检测
    json << "\"players\":[";
    for (size_t i = 0; i < data.players.size(); i++) {
        if (i > 0) json << ",";
        const auto& p = data.players[i];
        json << "{";
        json << "\"bbox\":{";
        json << "\"x\":" << p.bbox.x << ",";
        json << "\"y\":" << p.bbox.y << ",";
        json << "\"width\":" << p.bbox.width << ",";
        json << "\"height\":" << p.bbox.height;
        json << "},";
        json << "\"classId\":" << p.classId << ",";
        json << "\"confidence\":" << std::fixed << std::setprecision(4) << p.confidence << ",";
        json << "\"label\":\"" << p.label << "\"";
        
        // 添加球队ID（如果有）
        if (i < data.teamIds.size()) {
            json << ",\"teamId\":" << data.teamIds[i];
        }
        
        // 添加战术地图坐标（如果有）
        if (i < data.tacMapPositions.size()) {
            json << ",\"tacMapPosition\":{";
            json << "\"x\":" << std::fixed << std::setprecision(2) << data.tacMapPositions[i].x << ",";
            json << "\"y\":" << std::fixed << std::setprecision(2) << data.tacMapPositions[i].y;
            json << "}";
        }
        
        json << "}";
    }
    json << "],";
    
    // 关键点检测
    json << "\"keypoints\":[";
    for (size_t i = 0; i < data.keypoints.size(); i++) {
        if (i > 0) json << ",";
        const auto& k = data.keypoints[i];
        json << "{";
        json << "\"label\":\"" << k.label << "\",";
        json << "\"x\":" << std::fixed << std::setprecision(2) << k.center.x << ",";
        json << "\"y\":" << std::fixed << std::setprecision(2) << k.center.y << ",";
        json << "\"confidence\":" << std::fixed << std::setprecision(4) << k.confidence;
        json << "}";
    }
    json << "],";
    
    // 球检测
    json << "\"balls\":[";
    for (size_t i = 0; i < data.balls.size(); i++) {
        if (i > 0) json << ",";
        const auto& b = data.balls[i];
        json << "{";
        json << "\"x\":" << std::fixed << std::setprecision(2) << b.center.x << ",";
        json << "\"y\":" << std::fixed << std::setprecision(2) << b.center.y << ",";
        json << "\"confidence\":" << std::fixed << std::setprecision(4) << b.confidence;
        json << "}";
    }
    json << "]";
    
    json << "}";
    
    return json.str();
}

bool ApiClient::sendPostRequest(const std::string& endpoint, const std::string& jsonData) {
    try {
        httplib::Client client(host_, port_);
        client.set_connection_timeout(timeoutSeconds_, 0);
        client.set_read_timeout(timeoutSeconds_, 0);
        client.set_write_timeout(timeoutSeconds_, 0);
        
        httplib::Headers headers;
        headers.emplace("Content-Type", "application/json");
        
        if (!apiKey_.empty()) {
            headers.emplace("Authorization", "Bearer " + apiKey_);
        }
        
        std::string fullPath = basePath_ + endpoint;
        
        if (debugMode_) {
            std::cout << "POST " << host_ << ":" << port_ << fullPath << std::endl;
            std::cout << "Body: " << jsonData.substr(0, 200) << "..." << std::endl;
        }
        
        auto result = client.Post(fullPath.c_str(), headers, jsonData, "application/json");
        
        if (result) {
            if (debugMode_) {
                std::cout << "Response: " << result->status << std::endl;
            }
            return result->status >= 200 && result->status < 300;
        } else {
            std::cerr << "HTTP request failed" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "API request error: " << e.what() << std::endl;
        return false;
    }
}

bool ApiClient::sendFrameData(const FrameData& data) {
    std::string jsonData = frameDataToJson(data);
    return sendPostRequest("/api/frames", jsonData);
}

bool ApiClient::sendBatchFrameData(const std::vector<FrameData>& dataList) {
    std::ostringstream json;
    json << "{\"frames\":[";
    
    for (size_t i = 0; i < dataList.size(); i++) {
        if (i > 0) json << ",";
        json << frameDataToJson(dataList[i]);
    }
    
    json << "]}";
    
    return sendPostRequest("/api/frames/batch", json.str());
}

bool ApiClient::notifyVideoStart(const std::string& videoSource, int totalFrames) {
    std::ostringstream json;
    json << "{";
    json << "\"videoSource\":\"" << videoSource << "\",";
    json << "\"totalFrames\":" << totalFrames << ",";
    json << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    json << "}";
    
    return sendPostRequest("/api/video/start", json.str());
}

bool ApiClient::notifyVideoComplete(const std::string& videoSource) {
    std::ostringstream json;
    json << "{";
    json << "\"videoSource\":\"" << videoSource << "\",";
    json << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    json << "}";
    
    return sendPostRequest("/api/video/complete", json.str());
}

bool ApiClient::testConnection() {
    try {
        httplib::Client client(host_, port_);
        client.set_connection_timeout(5, 0);
        
        std::string fullPath = basePath_ + "/api/health";
        auto result = client.Get(fullPath.c_str());
        
        if (result) {
            std::cout << "API connection test successful (status: " 
                     << result->status << ")" << std::endl;
            return result->status >= 200 && result->status < 300;
        } else {
            std::cerr << "API connection test failed" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Connection test error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace FootballAnalytics
