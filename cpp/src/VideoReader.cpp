#include "VideoReader.h"
#include <iostream>
#include <stdexcept>

namespace FootballAnalytics {

VideoReader::VideoReader(const std::string& videoPath)
    : formatCtx_(nullptr)
    , codecCtx_(nullptr)
    , swsCtx_(nullptr)
    , frame_(nullptr)
    , frameRGB_(nullptr)
    , packet_(nullptr)
    , videoStreamIdx_(-1)
    , width_(0)
    , height_(0)
    , fps_(0.0)
    , totalFrames_(0)
    , currentFrameNumber_(0)
    , buffer_(nullptr)
{
    if (!initialize(videoPath)) {
        cleanup();
        throw std::runtime_error("Failed to initialize VideoReader for: " + videoPath);
    }
}

VideoReader::~VideoReader() {
    cleanup();
}

bool VideoReader::initialize(const std::string& videoPath) {
    // 打开输入文件
    if (avformat_open_input(&formatCtx_, videoPath.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Could not open video file: " << videoPath << std::endl;
        return false;
    }
    
    // 获取流信息
    if (avformat_find_stream_info(formatCtx_, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        return false;
    }
    
    // 查找视频流
    videoStreamIdx_ = -1;
    for (unsigned int i = 0; i < formatCtx_->nb_streams; i++) {
        if (formatCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIdx_ = i;
            break;
        }
    }
    
    if (videoStreamIdx_ == -1) {
        std::cerr << "Could not find video stream" << std::endl;
        return false;
    }
    
    // 获取解码器参数
    AVCodecParameters* codecParams = formatCtx_->streams[videoStreamIdx_]->codecpar;
    
    // 查找解码器
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Unsupported codec" << std::endl;
        return false;
    }
    
    // 创建解码器上下文
    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return false;
    }
    
    // 复制解码器参数
    if (avcodec_parameters_to_context(codecCtx_, codecParams) < 0) {
        std::cerr << "Could not copy codec parameters" << std::endl;
        return false;
    }
    
    // 打开解码器
    if (avcodec_open2(codecCtx_, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return false;
    }
    
    // 获取视频信息
    width_ = codecCtx_->width;
    height_ = codecCtx_->height;
    
    AVStream* videoStream = formatCtx_->streams[videoStreamIdx_];
    fps_ = av_q2d(videoStream->r_frame_rate);
    totalFrames_ = videoStream->nb_frames;
    
    // 如果总帧数未知，估算
    if (totalFrames_ <= 0 && videoStream->duration != AV_NOPTS_VALUE) {
        totalFrames_ = static_cast<int>(videoStream->duration * fps_ / AV_TIME_BASE);
    }
    
    // 分配帧结构
    frame_ = av_frame_alloc();
    frameRGB_ = av_frame_alloc();
    packet_ = av_packet_alloc();
    
    if (!frame_ || !frameRGB_ || !packet_) {
        std::cerr << "Could not allocate frame or packet" << std::endl;
        return false;
    }
    
    // 为RGB帧分配缓冲区
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width_, height_, 1);
    buffer_ = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
    
    av_image_fill_arrays(frameRGB_->data, frameRGB_->linesize, buffer_,
                        AV_PIX_FMT_BGR24, width_, height_, 1);
    
    // 创建转换上下文（YUV to BGR for OpenCV）
    swsCtx_ = sws_getContext(width_, height_, codecCtx_->pix_fmt,
                             width_, height_, AV_PIX_FMT_BGR24,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);
    
    if (!swsCtx_) {
        std::cerr << "Could not initialize conversion context" << std::endl;
        return false;
    }
    
    std::cout << "Video opened successfully:" << std::endl;
    std::cout << "  Resolution: " << width_ << "x" << height_ << std::endl;
    std::cout << "  FPS: " << fps_ << std::endl;
    std::cout << "  Total frames: " << totalFrames_ << std::endl;
    
    return true;
}

bool VideoReader::readFrame(cv::Mat& frame) {
    while (av_read_frame(formatCtx_, packet_) >= 0) {
        // 检查是否是视频流的包
        if (packet_->stream_index == videoStreamIdx_) {
            // 发送包到解码器
            int ret = avcodec_send_packet(codecCtx_, packet_);
            if (ret < 0) {
                std::cerr << "Error sending packet for decoding" << std::endl;
                av_packet_unref(packet_);
                continue;
            }
            
            // 接收解码后的帧
            ret = avcodec_receive_frame(codecCtx_, frame_);
            if (ret == 0) {
                // 成功解码帧
                // 转换为BGR格式（OpenCV格式）
                sws_scale(swsCtx_, frame_->data, frame_->linesize,
                         0, height_, frameRGB_->data, frameRGB_->linesize);
                
                // 创建OpenCV Mat
                frame = cv::Mat(height_, width_, CV_8UC3, frameRGB_->data[0],
                               frameRGB_->linesize[0]).clone();
                
                currentFrameNumber_++;
                av_packet_unref(packet_);
                return true;
            }
            else if (ret == AVERROR(EAGAIN)) {
                // 需要更多数据
                av_packet_unref(packet_);
                continue;
            }
            else if (ret == AVERROR_EOF) {
                // 文件结束
                av_packet_unref(packet_);
                return false;
            }
            else {
                std::cerr << "Error receiving frame from decoder" << std::endl;
                av_packet_unref(packet_);
                return false;
            }
        }
        av_packet_unref(packet_);
    }
    
    return false;
}

bool VideoReader::seekToFrame(int frameNumber) {
    if (frameNumber < 0 || frameNumber >= totalFrames_) {
        return false;
    }
    
    int64_t timestamp = static_cast<int64_t>(frameNumber * AV_TIME_BASE / fps_);
    
    if (av_seek_frame(formatCtx_, videoStreamIdx_, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        std::cerr << "Error seeking to frame " << frameNumber << std::endl;
        return false;
    }
    
    avcodec_flush_buffers(codecCtx_);
    currentFrameNumber_ = frameNumber;
    
    return true;
}

void VideoReader::cleanup() {
    if (buffer_) {
        av_free(buffer_);
        buffer_ = nullptr;
    }
    
    if (frameRGB_) {
        av_frame_free(&frameRGB_);
        frameRGB_ = nullptr;
    }
    
    if (frame_) {
        av_frame_free(&frame_);
        frame_ = nullptr;
    }
    
    if (packet_) {
        av_packet_free(&packet_);
        packet_ = nullptr;
    }
    
    if (swsCtx_) {
        sws_freeContext(swsCtx_);
        swsCtx_ = nullptr;
    }
    
    if (codecCtx_) {
        avcodec_free_context(&codecCtx_);
        codecCtx_ = nullptr;
    }
    
    if (formatCtx_) {
        avformat_close_input(&formatCtx_);
        formatCtx_ = nullptr;
    }
}

} // namespace FootballAnalytics
