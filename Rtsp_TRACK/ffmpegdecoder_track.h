//Yasar Utku Alcalar
#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <iostream>
#include <string>
#include <QString>
#include <thread>
#include <mutex>
#include <chrono>
#include <opencv2/opencv.hpp>

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>
}

class FFmpegDecoder
{
    public:
        FFmpegDecoder(std::string);
        ~FFmpegDecoder();

        int initial(QString & url);
        void connect();
        void decode();

        bool isConnected() const {return bConnected;}

        std::deque <cv::Mat> decodedImgBuf;
        std::mutex mtx;

    private:

        void destroy();

        AVFormatContext *pFormatCtx;
        AVCodecContext *pCodecCtx;
        AVCodec *pCodec;
        AVFrame *pFrame, *pFrameBGR;
        AVPacket *packet;
        uint8_t *outBuffer;
        SwsContext *imgConvertCtx;

        int videoStream;

        std::string path;
        QString rtspURL;

        bool bConnected;
};
#endif // FFMPEGDECODER_H
