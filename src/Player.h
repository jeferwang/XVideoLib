#pragma once

#include "Base.h"
#include <memory>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>

typedef std::function<void(void *TextureDataPtr, int64_t TextureIndex, int64_t FrameIndex, double_t FrameShowTime, int32_t Width, int32_t Height)> VideoFrameUpdateCallback;

struct AVFormatContext;
struct AVStream;
struct AVCodec;
struct AVCodecContext;
struct AVCodecParserContext;
struct AVPacket;
struct AVFrame;
struct SwsContext;
struct AVBufferRef;

namespace XVideo {

    using std::chrono::duration;
    using std::chrono::time_point;
    using std::chrono::steady_clock;

    class XVIDEO_API Player final {
    public:
        Player();

        ~Player();

        // 初始化，打开指定视频文件输入，重复调用无效
        bool Init(const char *InUrl, uint8_t InAVHWDeviceType);

        // 执行清理，可重复调用
        void UnInit();

        bool Play();

        void Pause();

        void SetLoop(bool InLoop);

        void SetFrameUpdateCallback(const VideoFrameUpdateCallback &InCallback);

        int32_t GetWidth() const;

        int32_t GetHeight() const;

        bool IsUseHwDevice() const;

    private:
        void DoDecode();

        void PlaybackTimer();

        double_t FrameIndexToTime(int64_t InFrameIndex, int64_t InFrameRate);

        int64_t TimeToFrameIndex(double_t InTimeSeconds, int64_t InFrameRate);

        int64_t StreamFrameRate();

        static AVCodec *DetectCodec(int32_t CodecID);

        static AVStream *FindVideoStream(AVFormatContext *InFormatContext);

        int32_t GetHwFormat(AVCodecContext *InCodecContext, const int32_t *InPixFormats);

    private:
        bool Init_ = false;
        std::vector<VideoFrameUpdateCallback> FrameUpdateCallbacks_;

        // 硬解
        bool UseHwDevice = false;
        uint8_t HWDeviceType = 0;//AVHWDeviceType
        int32_t HwPixFmt = -1; // AVPixelFormat
        AVBufferRef *HwDeviceCtx = nullptr;
        // AVFrame *HwFrame_ = nullptr;

        // 解码线程
        std::shared_ptr<std::thread> DecodeThread_ = nullptr;
        std::mutex PlaybackTimeMutex_; // 修改PlaybackTime的时候需要加锁
        std::shared_ptr<std::thread> TimerThread_ = nullptr;

        // Playback Control
        bool Start_ = false;// 解码状态，启动和停止
        bool Playing_ = false;// 播放状态，播放和暂停
        bool Loop_ = false;
        double_t PlaybackTime_ = 0;// 播放进度

        // Decode params
        AVFormatContext *FormatContext_ = nullptr;
        AVStream *Stream_ = nullptr;
        AVCodecContext *CodecContext_ = nullptr;
        AVPacket *Packet_ = nullptr;
        AVFrame *Frame_ = nullptr;
        SwsContext *SwsContext_ = nullptr;
        int FrameWidth_ = 0;
        int FrameHeight_ = 0;
        double_t FrameShowTime_ = 0;// 单位：秒
        uint8_t *ImageData_[4] = {nullptr, nullptr, nullptr, nullptr};
        int ImageLineSize[4] = {0, 0, 0, 0};

        int64_t StreamTimeBase();
    };
}
