#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

#include "VideoReader.h"
#include "YOLODetector.h"
#include "TeamPredictor.h"
#include "CoordinateTransform.h"
#include "ApiClient.h"

using namespace FootballAnalytics;

/**
 * @brief 显示使用说明
 */
void printUsage(const char* programName) {
    std::cout << "Football Analytics C++ Application" << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --video <path>              Input video file path (required)" << std::endl;
    std::cout << "  --api-url <url>             API server URL (default: http://localhost:8080)" << std::endl;
    std::cout << "  --api-key <key>             API authentication key (optional)" << std::endl;
    std::cout << "  --player-model <path>       Player detection model path (default: ./models/players.onnx)" << std::endl;
    std::cout << "  --keypoint-model <path>     Keypoint detection model path (default: ./models/keypoints.onnx)" << std::endl;
    std::cout << "  --player-conf <value>       Player detection confidence threshold (default: 0.6)" << std::endl;
    std::cout << "  --keypoint-conf <value>     Keypoint detection confidence threshold (default: 0.7)" << std::endl;
    std::cout << "  --team1-name <name>         First team name (default: Team1)" << std::endl;
    std::cout << "  --team2-name <name>         Second team name (default: Team2)" << std::endl;
    std::cout << "  --debug                     Enable debug mode" << std::endl;
    std::cout << "  --help                      Show this help message" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 解析命令行参数
 */
struct Config {
    std::string videoPath;
    std::string apiUrl = "http://localhost:8080";
    std::string apiKey;
    std::string playerModelPath = "./models/players.onnx";
    std::string keypointModelPath = "./models/keypoints.onnx";
    std::string tacticalMapPath = "./resources/tactical_map.jpg";
    std::string keypointMapPath = "./config/pitch_map_labels.json";
    float playerConfThreshold = 0.6f;
    float keypointConfThreshold = 0.7f;
    std::string team1Name = "Team1";
    std::string team2Name = "Team2";
    bool debugMode = false;
};

bool parseArguments(int argc, char** argv, Config& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            return false;
        } else if (arg == "--video" && i + 1 < argc) {
            config.videoPath = argv[++i];
        } else if (arg == "--api-url" && i + 1 < argc) {
            config.apiUrl = argv[++i];
        } else if (arg == "--api-key" && i + 1 < argc) {
            config.apiKey = argv[++i];
        } else if (arg == "--player-model" && i + 1 < argc) {
            config.playerModelPath = argv[++i];
        } else if (arg == "--keypoint-model" && i + 1 < argc) {
            config.keypointModelPath = argv[++i];
        } else if (arg == "--player-conf" && i + 1 < argc) {
            config.playerConfThreshold = std::stof(argv[++i]);
        } else if (arg == "--keypoint-conf" && i + 1 < argc) {
            config.keypointConfThreshold = std::stof(argv[++i]);
        } else if (arg == "--team1-name" && i + 1 < argc) {
            config.team1Name = argv[++i];
        } else if (arg == "--team2-name" && i + 1 < argc) {
            config.team2Name = argv[++i];
        } else if (arg == "--debug") {
            config.debugMode = true;
        }
    }
    
    return !config.videoPath.empty();
}

/**
 * @brief 主函数
 */
int main(int argc, char** argv) {
    std::cout << "==================================================" << std::endl;
    std::cout << "  Football Analytics with Deep Learning & CV" << std::endl;
    std::cout << "  C++ Implementation" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;
    
    // 解析命令行参数
    Config config;
    if (!parseArguments(argc, argv, config)) {
        printUsage(argv[0]);
        return config.videoPath.empty() ? 1 : 0;
    }
    
    try {
        // 1. 初始化视频读取器
        std::cout << "[1/7] Initializing video reader..." << std::endl;
        VideoReader videoReader(config.videoPath);
        
        if (!videoReader.isOpened()) {
            std::cerr << "Failed to open video: " << config.videoPath << std::endl;
            return 1;
        }
        
        // 2. 初始化YOLO检测器
        std::cout << "[2/7] Loading detection models..." << std::endl;
        YOLODetector playerDetector(config.playerModelPath, config.playerConfThreshold);
        YOLODetector keypointDetector(config.keypointModelPath, config.keypointConfThreshold);
        
        // 设置类别标签
        std::vector<std::string> playerLabels = {"player", "referee", "ball"};
        playerDetector.setClassLabels(playerLabels);
        
        // 3. 初始化球队预测器
        std::cout << "[3/7] Initializing team predictor..." << std::endl;
        TeamPredictor teamPredictor(3); // 提取3种主要颜色
        
        // 设置球队颜色（示例颜色，实际应从配置读取）
        TeamColorInfo team1(config.team1Name, 
                           cv::Scalar(30, 37, 48),    // 深蓝色
                           cv::Scalar(245, 253, 21)); // 黄色
        TeamColorInfo team2(config.team2Name,
                           cv::Scalar(251, 252, 250),  // 白色
                           cv::Scalar(177, 252, 196)); // 浅绿色
        teamPredictor.setTeamColors(team1, team2);
        
        // 4. 初始化坐标转换器
        std::cout << "[4/7] Initializing coordinate transform..." << std::endl;
        CoordinateTransform coordTransform(config.keypointMapPath, 7.0f);
        coordTransform.loadTacticalMap(config.tacticalMapPath);
        
        // 5. 初始化API客户端
        std::cout << "[5/7] Connecting to API server..." << std::endl;
        ApiClient apiClient(config.apiUrl, config.apiKey);
        apiClient.setDebug(config.debugMode);
        
        // 测试API连接
        if (!apiClient.testConnection()) {
            std::cerr << "Warning: Failed to connect to API server" << std::endl;
            std::cout << "Continue anyway? (y/n): ";
            char response;
            std::cin >> response;
            if (response != 'y' && response != 'Y') {
                return 1;
            }
        }
        
        // 通知视频处理开始
        apiClient.notifyVideoStart(config.videoPath, videoReader.getTotalFrames());
        
        // 6. 处理视频帧
        std::cout << "[6/7] Processing video frames..." << std::endl;
        std::cout << "Total frames: " << videoReader.getTotalFrames() << std::endl;
        std::cout << std::endl;
        
        cv::Mat frame;
        int frameNumber = 0;
        int processedFrames = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        while (videoReader.readFrame(frame)) {
            frameNumber++;
            
            auto frameStartTime = std::chrono::high_resolution_clock::now();
            
            // 创建帧数据
            FrameData frameData;
            frameData.frameNumber = frameNumber;
            frameData.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            frameData.videoSource = config.videoPath;
            
            // 检测球员和球
            auto playerDetections = playerDetector.detect(frame);
            
            // 分离球员和球
            for (const auto& det : playerDetections) {
                if (det.classId == 0) { // 球员
                    frameData.players.push_back(det);
                } else if (det.classId == 2) { // 球
                    frameData.balls.push_back(det);
                }
            }
            
            // 检测球场关键点
            frameData.keypoints = keypointDetector.detect(frame);
            
            // 计算单应性矩阵
            if (frameData.keypoints.size() >= 4) {
                coordTransform.computeHomography(frameData.keypoints, frameNumber);
            }
            
            // 球队预测
            if (!frameData.players.empty()) {
                frameData.teamIds = teamPredictor.predictTeams(frame, frameData.players);
            }
            
            // 坐标转换
            if (coordTransform.hasValidHomography() && !frameData.players.empty()) {
                frameData.tacMapPositions = coordTransform.transformToTacticalMap(frameData.players);
            }
            
            // 发送数据到API
            if (!apiClient.sendFrameData(frameData)) {
                std::cerr << "Warning: Failed to send frame " << frameNumber << " data" << std::endl;
            }
            
            processedFrames++;
            
            // 计算并显示进度
            auto frameEndTime = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                frameEndTime - frameStartTime).count();
            
            if (frameNumber % 30 == 0 || frameNumber == videoReader.getTotalFrames()) {
                float progress = (float)frameNumber / videoReader.getTotalFrames() * 100.0f;
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    frameEndTime - startTime).count();
                float fps = (float)processedFrames / elapsed;
                
                std::cout << "\rProgress: " << std::fixed << std::setprecision(1) << progress << "% "
                         << "(" << frameNumber << "/" << videoReader.getTotalFrames() << ") "
                         << "| FPS: " << std::setprecision(2) << fps << " "
                         << "| Frame time: " << frameDuration << "ms    " << std::flush;
            }
        }
        
        std::cout << std::endl;
        
        // 7. 完成处理
        std::cout << "[7/7] Finalizing..." << std::endl;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration_cast<std::chrono::seconds>(
            endTime - startTime).count();
        
        // 通知视频处理完成
        apiClient.notifyVideoComplete(config.videoPath);
        
        // 显示统计信息
        std::cout << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << "  Processing Complete!" << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << "Total frames processed: " << processedFrames << std::endl;
        std::cout << "Total time: " << totalDuration << " seconds" << std::endl;
        std::cout << "Average FPS: " << std::fixed << std::setprecision(2) 
                 << (float)processedFrames / totalDuration << std::endl;
        std::cout << "==================================================" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
