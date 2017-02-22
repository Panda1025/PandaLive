//
// Created by panda on 2017/2/21.
//
#include <android/bitmap.h>
#include <android/log.h>
#include <libavcodec/avcodec.h>
#include <setjmp.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include "libjpeg-turbo/jpeglib.h"
typedef uint8_t BYTE;
#define TAG "image "
#define LOG_TAG "FFmpeg"
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, format, ##__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

AVFormatContext *ofmt_ctx = NULL;
AVStream *out_stream = NULL;
AVPacket pkt;
AVCodecContext *pCodecCtx = NULL;
AVCodec *pCodec = NULL;
AVFrame *yuv_frame;

int frame_count;
int src_width;
int src_height;
int y_length;
int uv_length;
int64_t start_time;

#define true 1
#define false 0
char *error;
struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    error=myerr->pub.jpeg_message_table[myerr->pub.msg_code];
    LOGE("jpeg_message_table[%d]:%s", myerr->pub.msg_code,myerr->pub.jpeg_message_table[myerr->pub.msg_code]);
    // LOGE("addon_message_table:%s", myerr->pub.addon_message_table);
//  LOGE("SIZEOF:%d",myerr->pub.msg_parm.i[0]);
//  LOGE("sizeof:%d",myerr->pub.msg_parm.i[1]);
    longjmp(myerr->setjmp_buffer, 1);
}
//图片压缩方法
int generateJPEG(BYTE* data, int w, int h, int quality,
                 const char* outfilename, jboolean optimize) {
    int nComponent = 3;

    struct jpeg_compress_struct jcs;

    struct my_error_mgr jem;

    jcs.err = jpeg_std_error(&jem.pub);
    jem.pub.error_exit = my_error_exit;
    if (setjmp(jem.setjmp_buffer)) {
        return 0;
    }
    //为JPEG对象分配空间并初始化
    jpeg_create_compress(&jcs);
    //获取文件信息
    FILE* f = fopen(outfilename, "wb");
    if (f == NULL) {
        return 0;
    }
    //指定压缩数据源
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;
    jcs.image_height = h;
    if (optimize) {
        LOGE("optimize==ture");
    } else {
        LOGE("optimize==false");
    }

    jcs.arith_code = false;
    jcs.input_components = nComponent;
    if (nComponent == 1)
        jcs.in_color_space = JCS_GRAYSCALE;
    else
        jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);
    jcs.optimize_coding = optimize;
    //为压缩设定参数，包括图像大小，颜色空间
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride;
    row_stride = jcs.image_width * nComponent;
    while (jcs.next_scanline < jcs.image_height) {
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        //写入数据
        jpeg_write_scanlines(&jcs, row_pointer, 1);
    }

    if (jcs.optimize_coding) {
        LOGE("optimize==ture");
    } else {
        LOGE("optimize==false");
    }
    //压缩完毕
    jpeg_finish_compress(&jcs);
    //释放资源
    jpeg_destroy_compress(&jcs);
    fclose(f);

    return 1;
}

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

//将java string转换为char*
char* jstringTostring(JNIEnv* env, jbyteArray barr) {
    char* rtn = NULL;
    jsize alen = (*env)->GetArrayLength(env, barr);
    jbyte* ba = (*env)->GetByteArrayElements(env, barr, 0);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    (*env)->ReleaseByteArrayElements(env, barr, ba, 0);
    return rtn;
}

JNIEXPORT jstring JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_helloFromFFmpeg(JNIEnv *env, jobject type) {
    // TODO
    char info[10000] = {0};
    sprintf(info, "%s\n", avcodec_configuration());
    return (*env)->NewStringUTF(env, info);
}



JNIEXPORT jstring JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_compressBitmap(JNIEnv *env, jobject type, jobject bitmap,
                                                       jint width, jint height, jint quality,
                                                       jbyteArray fileName, jboolean optimize) {
    AndroidBitmapInfo infoColor;
    int ret;
    BYTE *pixelColor;
    BYTE *data;
    BYTE *tempData;
    char *filename = jstringTostring(env,fileName);


    if((ret = AndroidBitmap_getInfo(env,bitmap,&infoColor)) < 0) {\
        LOGE("解析错误");
        return (*env)->NewStringUTF(env,"1");
    }

    if((ret = AndroidBitmap_lockPixels(env,bitmap,&pixelColor)) < 0) {
        LOGE("加载失败");
    }

    BYTE r,g,b;
    int color;
    data = malloc(width* height * 3);
    tempData = data;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            color = *((int*)pixelColor);
            r = ((color&0x00FF0000) >> 16);
            g = ((color&0x0000FF00) >> 8);
            b = color&0X000000FF;

            *data = b;
            *(data+1) = g;
            *(data+2) = r;
            data += 3;
            pixelColor += 4;
        }
    }

    AndroidBitmap_unlockPixels(env,bitmap);
    int resultCode = generateJPEG(tempData,width,height,quality,filename,optimize);

    free(tempData);
    if(resultCode == 0) {
        jstring result = (*env)->NewStringUTF(env,"0");
        return result;
    }

    return (*env)->NewStringUTF(env,"1");
}

/**
 * 回调函数，用来把FFmpeg的log写到sdcard里面
 */
void live_log(void *ptr, int level, const char* fmt, va_list vl) {
    FILE *fp = fopen("/sdcard/123/live_log.txt", "a+");
    if(fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}

/**
 * 编码函数
 * avcodec_encode_video2被deprecated后，自己封装的
 */
int encode(AVCodecContext *pCodecCtx, AVPacket* pPkt, AVFrame *pFrame, int *got_packet) {
    int ret;

    *got_packet = 0;

    ret = avcodec_send_frame(pCodecCtx, pFrame);
    if(ret <0 && ret != AVERROR_EOF) {
        return ret;
    }

    ret = avcodec_receive_packet(pCodecCtx, pPkt);
    if(ret < 0 && ret != AVERROR(EAGAIN)) {
        return ret;
    }

    if(ret >= 0) {
        *got_packet = 1;
    }

    return 0;
}





JNIEXPORT jint JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_streamerInit(JNIEnv *env, jclass type, jint width,
                                                     jint height, jstring addr) {
    // TODO
    int ret = 0;
    const char *address = (*env)->GetStringUTFChars(env, addr, 0);

    src_width = width;
    src_height = height;
    //yuv数据格式里面的  y的大小（占用的空间）
    y_length = width * height;
    //u/v占用的空间大小
    uv_length = y_length / 4;

    //设置回调函数，写log
    av_log_set_callback(live_log);

    //激活所有的功能
    av_register_all();

    //推流就需要初始化网络协议
    avformat_network_init();

    //初始化AVFormatContext
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", address);
    if(!ofmt_ctx) {
        LOGE("Could not create output context\n");
        return -1;
    }

    //寻找编码器，这里用的就是x264的那个编码器了
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!pCodec) {
        LOGE("Can not find encoder!\n");
        return -1;
    }

    //初始化编码器的context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;  //指定编码格式
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;
    pCodecCtx->bit_rate = 800000;
    pCodecCtx->gop_size = 300;

    if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;

    pCodecCtx->max_b_frames = 3;

    AVDictionary *dicParams = NULL;
    av_dict_set(&dicParams, "preset", "ultrafast", 0);
    av_dict_set(&dicParams, "tune", "zerolatency", 0);

    //打开编码器
    if(avcodec_open2(pCodecCtx, pCodec, &dicParams) < 0) {
        LOGE("Failed to open encoder!\n");
        return -1;
    }

    //新建输出流
    out_stream = avformat_new_stream(ofmt_ctx, pCodec);
    if(!out_stream) {
        LOGE("Failed allocation output stream\n");
        return -1;
    }
    out_stream->time_base.num = 1;
    out_stream->time_base.den = 30;
    //复制一份编码器的配置给输出流
    avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx);

    //打开输出流
    ret = avio_open(&ofmt_ctx->pb, address, AVIO_FLAG_WRITE);
    if(ret < 0) {
        LOGE("Could not open output URL %s", address);
        return -1;
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if(ret < 0) {
        LOGE("Error occurred when open output URL\n");
        return -1;
    }

    //初始化一个帧的数据结构，用于编码用
    //指定AV_PIX_FMT_YUV420P这种格式的
    yuv_frame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(av_image_get_buffer_size(pCodecCtx->pix_fmt, src_width, src_height, 1));
    av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, out_buffer, pCodecCtx->pix_fmt, src_width, src_height, 1);

    start_time = av_gettime();

    return 0;
}

JNIEXPORT jint JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_streamerHandle(JNIEnv *env, jclass type, jbyteArray data_) {
    jbyte *data = (*env)->GetByteArrayElements(env, data_, NULL);

    // TODO
    int ret, i, resultCode;
    int got_packet = 0;
    resultCode = 0;

    /**
     * 这里就是之前说的NV21转为AV_PIX_FMT_YUV420P这种格式的操作了
     */
    memcpy(yuv_frame->data[0], data, y_length);
    for (i = 0; i < uv_length; i++) {
        *(yuv_frame->data[2] + i) = *(data + y_length + i * 2);
        *(yuv_frame->data[1] + i) = *(data + y_length + i * 2 + 1);
    }

    yuv_frame->format = pCodecCtx->pix_fmt;
    yuv_frame->width = src_width;
    yuv_frame->height = src_height;
    //yuv_frame->pts = frame_count;
    yuv_frame->pts = (1.0 / 30) * 90 * frame_count;


    pkt.data = NULL;
    pkt.size = 0;
    av_init_packet(&pkt);

    //进行编码
    ret = encode(pCodecCtx, &pkt, yuv_frame, &got_packet);
    if(ret < 0) {
        resultCode = -1;
        LOGE("Encode error\n");
        goto end;
    }
    if(got_packet) {
        LOGI("Encode frame: %d\tsize:%d\n", frame_count, pkt.size);
        frame_count++;
        pkt.stream_index = out_stream->index;

        //写PTS/DTS
        AVRational time_base1 = ofmt_ctx->streams[0]->time_base;
        AVRational r_frame_rate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};
        int64_t calc_duration = (double)(AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));

        pkt.pts = av_rescale_q(frame_count * calc_duration, time_base_q, time_base1);
        pkt.dts = pkt.pts;
        pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base1);
        pkt.pos = -1;

        //处理延迟
        int64_t pts_time = av_rescale_q(pkt.dts, time_base1, time_base_q);
        int64_t now_time = av_gettime() - start_time;
        if(pts_time > now_time) {
            av_usleep(pts_time - now_time);
        }

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if(ret < 0) {
            LOGE("Error muxing packet");
            resultCode = -1;
            goto end;
        }
        av_packet_unref(&pkt);
    }


    end:
    (*env)->ReleaseByteArrayElements(env, data_, data, 0);
    return resultCode;
}

JNIEXPORT jint JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_streamerFlush(JNIEnv *env, jobject instance) {

    // TODO
    int ret;
    int got_packet;
    AVPacket packet;
    if(!(pCodec->capabilities & CODEC_CAP_DELAY)) {
        return 0;
    }

    while(1) {
        packet.data = NULL;
        packet.size = 0;
        av_init_packet(&packet);
        ret = encode(pCodecCtx, &packet, NULL, &got_packet);
        if(ret < 0) {
            break;
        }
        if(!got_packet) {
            ret = 0;
            break;
        }

        LOGI("Encode 1 frame size:%d\n", packet.size);

        AVRational time_base = ofmt_ctx->streams[0]->time_base;
        AVRational r_frame_rate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};

        int64_t calc_duration = (double)(AV_TIME_BASE) * (1 / av_q2d(r_frame_rate1));

        packet.pts = av_rescale_q(frame_count * calc_duration, time_base_q, time_base);
        packet.dts = packet.pts;
        packet.duration = av_rescale_q(calc_duration, time_base_q, time_base);

        packet.pos = -1;
        frame_count++;
        ofmt_ctx->duration = packet.duration * frame_count;

        ret = av_interleaved_write_frame(ofmt_ctx, &packet);
        if(ret < 0) {
            break;
        }
    }

    //写文件尾
    av_write_trailer(ofmt_ctx);
    return 0;
}

JNIEXPORT jint JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_streamerRelease(JNIEnv *env, jobject instance) {

    // TODO
    if(pCodecCtx) {
        avcodec_close(pCodecCtx);
        pCodecCtx = NULL;
    }

    if(ofmt_ctx) {
        avio_close(ofmt_ctx->pb);
    }
    if(ofmt_ctx) {
        avformat_free_context(ofmt_ctx);
        ofmt_ctx = NULL;
    }

    if(yuv_frame) {
        av_frame_free(&yuv_frame);
        yuv_frame = NULL;
    }


}
