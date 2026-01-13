#pragma once

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace FootballAnalytics {

/**
 * @brief FFmpeg视频读取器类
 * 
 * 使用FFmpeg API读取视频流并抽取帧
 */
class VideoReader {
public:
    /**
     * @brief 构造函数
     * @param videoPath 视频文件路径
     */
    explicit VideoReader(const std::string& videoPath);
    
    /**
     * @brief 析构函数
     */
    ~VideoReader();
    
    // 禁止拷贝
    VideoReader(const VideoReader&) = delete;
    VideoReader& operator=(const VideoReader&) = delete;
    
    /**
     * @brief 读取下一帧
     * @param frame 输出的帧数据（OpenCV Mat格式）
     * @return 成功返回true，失败或到达文件末尾返回false
     */
    bool readFrame(cv::Mat& frame);
    
    /**
     * @brief 跳转到指定帧
     * @param frameNumber 目标帧号
     * @return 成功返回true，失败返回false
     */
    bool seekToFrame(int frameNumber);
    
    /**
     * @brief 获取视频宽度
     */
    int getFrameWidth() const { return width_; }
    
    /**
     * @brief 获取视频高度
     */
    int getFrameHeight() const { return height_; }
    
    /**
     * @brief 获取帧率
     */
    double getFPS() const { return fps_; }
    
    /**
     * @brief 获取总帧数
     */
    int getTotalFrames() const { return totalFrames_; }
    
    /**
     * @brief 检查是否成功打开
     */
    bool isOpened() const { return formatCtx_ != nullptr; }
    
    /**
     * @brief 获取当前帧号
     */
    int getCurrentFrameNumber() const { return currentFrameNumber_; }

private:
    AVFormatContext* formatCtx_;
    AVCodecContext* codecCtx_;
    SwsContext* swsCtx_;
    AVFrame* frame_;
    AVFrame* frameRGB_;
    AVPacket* packet_;
    
    int videoStreamIdx_;
    int width_;
    int height_;
    double fps_;
    int totalFrames_;
    int currentFrameNumber_;
    
    uint8_t* buffer_;
    
    /**
     * @brief 初始化FFmpeg相关结构
     */
    bool initialize(const std::string& videoPath);
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 解码帧
     */
    bool decodeFrame(cv::Mat& outputFrame);
};

} // namespace FootballAnalytics
