//
// Created by Huijun on 2019/6/12.
//

#include "FFmpegControl.h"

void *prepareFFmpeg_(void *args) {
    // 无法访问对象的成员变量，这里通过pthread_create中第四个参数，将对象传进来
    // 将args强转为FFmpegControl对象
    FFmpegControl *ffmpegControl = static_cast<FFmpegControl *>(args);
    ffmpegControl->prepareFFmpeg();

    //
    return 0;
}

FFmpegControl::FFmpegControl(JavaCallHelper *pHelper, const char *dataSource) {
    this->javaCallHelper = pHelper;
    // 另开辟一个字符串
    url = new char[strlen(dataSource) + 1];
    strcpy(url, dataSource);
}

FFmpegControl::~FFmpegControl() {

}

void FFmpegControl::prepare() {
    // 第三个参数传一个函数，类似于java中thread的run方法
    // 第四个参数是对当前线程传递的参数，会传递到函数的args中
    pthread_create(&prepare_thread, NULL, prepareFFmpeg_, this);
}

void FFmpegControl::prepareFFmpeg() {
    // 在子线程中执行，能访问到对象的属性

    // 初始化网络模块
    avformat_network_init();
    // 获取总上下文，代表一个 视频/音频 包含了视频、音频的各种信息
    formatContext = avformat_alloc_context();

    AVDictionary *opts = NULL;
    // 设置超时时间3秒
    av_dict_set(&opts, "timeout", "3000000", 0);
    // 打开视频url
    int ret = avformat_open_input(&formatContext, url, NULL, &opts);
    if (ret != 0) {
        // 打开返回失败，反射java层
        if(javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMEPG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    // 查找流
    ret = avformat_find_stream_info(formatContext, NULL);
    if(ret != 0) {
        if(javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMEPG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        // 流的解码参数
        AVCodecParameters *codecParameters = stream->codecpar;
        // 找到解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if(!codec) {
            if(javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMEPG_FIND_DECODER_FAIL);
            }
            return;
        }

        // 创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if(!codecContext) {
            if(javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMEPG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }

        // 配置上下文参数
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if(ret < 0) {
            if(javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMEPG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }

        // 打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if(ret != 0) {
            if(javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMEPG_OPEN_DECODER_FAIL);
            }
            return;
        }

        if(codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            // 视频流
            videoChannel = new VideoChannel(0, codecContext, javaCallHelper, stream->time_base);
        } else if(codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            // 音频流
            audioChannel = new AudioChannel(i, javaCallHelper, codec);
        }
    }

    if(!videoChannel && !audioChannel) {
        if(javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMEPG_NOMEDIA);
        }
        return;
    }
    if(javaCallHelper) {
        javaCallHelper->onPrepare(THREAD_CHILD);
    }
}

void FFmpegControl::start() {
    isPlaying = true;
    if(videoChannel) {
        videoChannel->play();
    }
//    if(audioChannel) {
//        audioChannel->play();
//    }
}
