#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <zconf.h>

//指定混合编译模式，兼容C库
extern "C" {
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_xinyuan_study_mycmake_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_xyz_study_cusffmpegplayer_FFPlayer_native_1start(JNIEnv *env, jobject thiz, jstring path_,
                                                          jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);
    //ffmpeg 播放

    //初始化网络，播放远程地址时需要
    avformat_network_init();

    //    AVFormatContext             AVCodecContext           SwsContext
    //mp4------------------>h.264和aac---------------->YUV和PCM------------->SurfaceView

    //总上下文
    AVFormatContext *formatContext = avformat_alloc_context();

    //设置超时等参数
    AVDictionary *options = NULL;
    av_dict_set(&options, "timeout", "3000000", 0);
    //打开输入流，formatContext就有了视频流信息
    int rec = avformat_open_input(&formatContext, path, NULL, &options);
    if (rec) {
        return;
    }
    //流索引
    int video_stream_index = -1;
    int audio_stream_index = -1;
    //解析流信息
    avformat_find_stream_info(formatContext, NULL);
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
        } else if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
        }
    }
    //拿到解码出的视频流参数
    AVCodecParameters *codecPar = formatContext->streams[video_stream_index]->codecpar;

    //拿到视频流对应的解码器，通过id
    AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);

    //分配解码器上下文
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);

    //把解码出的视频流参数 复制给 解码器上下文
    avcodec_parameters_to_context(codecContext, codecPar);

    //通过codec初始化codecContext
    int rec1 = avcodec_open2(codecContext, codec, NULL);
    if (rec1 < 0) {
        return;
    }
    //使用解码器开始解码

    //分配一个AVPacket
    AVPacket *avPacket = av_packet_alloc();

    //分配一个转换上下文，下面会用。flags的作用是转化算法不同，有的是重视质量，有的是重视速度
    SwsContext *swsContext = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, 0, 0, 0);

    //底层都是通过ANativeWindow来进行图形绘制的
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width, codecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer outBuffer;

    //从流中读取下一帧(数据包)
    while (av_read_frame(formatContext, avPacket) >= 0) {
        //把avPacket喂给avCodec，进行解码
        avcodec_send_packet(codecContext, avPacket);
        //接收解码器解码出的源数据
        AVFrame *frame = av_frame_alloc();
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        //需要使用转码上下文SwsContext,将YUV数据转为RGBA数据
        //RGBA接收的容器
        uint8_t *dst_data[4];
        //每一行的首地址
        int dst_lineSizes[4];
        av_image_alloc(dst_data, dst_lineSizes, codecContext->width, codecContext->height,
                       AV_PIX_FMT_RGBA, 1);
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, dst_data,
                  dst_lineSizes);

        //锁住window
        ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
        // 绘制
        uint8_t *firstWindow = static_cast<uint8_t *>(outBuffer.bits);
        uint8_t *src_data = dst_data[0];
        //一行有多少个字节，一个ARGB是4个字节
        int dest_stride = outBuffer.stride * 4;
        int src_line_size = dst_lineSizes[0];

        for (int i = 0; i < outBuffer.height; ++i) {
            memcpy(firstWindow + i * dest_stride, src_data + i * src_line_size, dest_stride);
        }
        //解锁
        ANativeWindow_unlockAndPost(nativeWindow);
        usleep(1000 * 16);
        av_frame_free(&frame);
    }

    ANativeWindow_release(nativeWindow);
    avcodec_close(codecContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(path_, path);
}