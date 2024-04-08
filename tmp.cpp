extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>

}

int main(int argc, char* argv[])
{
    AVFormatContext* formatContext = NULL;
    AVOutputFormat* outputFormat = NULL;
    AVStream* outStream = NULL;
    const AVCodec* codec = NULL;
    AVCodecContext* codecContext = NULL;
    int ret;

    // 初始化Libav
//    av_register_all();
//    avformat_network_init();

    // 创建输出格式上下文
    ret = avformat_alloc_output_context2(&formatContext, NULL, "rtp", "rtp://127.0.0.1:1234");
    if (ret < 0) {
        fprintf(stderr, "Could not allocate output format context\n");
        return 1;
    }

//    outputFormat = formatContext->oformat;

    // 创建视频流
    outStream = avformat_new_stream(formatContext, NULL);
    if (!outStream) {
        fprintf(stderr, "Could not create video stream\n");
        return 1;
    }

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Could not find H.264 encoder\n");
        return 1;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
    }

    codecContext->codec_id = codec->id;
    codecContext->bit_rate = 400000;
    codecContext->width = 640;
    codecContext->height = 480;
    codecContext->time_base = (AVRational) {1, 25};
    codecContext->gop_size = 10;
    codecContext->max_b_frames = 1;

    if (outputFormat->flags & AVFMT_GLOBALHEADER) {
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(codecContext, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
        return 1;
    }

    outStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    outStream->codecpar->codec_id = AV_CODEC_ID_H264;
    outStream->codecpar->format = codecContext->pix_fmt;
    outStream->codecpar->width = codecContext->width;
    outStream->codecpar->height = codecContext->height;
    outStream->codecpar->bit_rate = codecContext->bit_rate;
    outStream->codecpar->extradata_size = codecContext->extradata_size;
    outStream->codecpar->extradata = codecContext->extradata;

    av_dump_format(formatContext, 0, "rtp", 1);

    // 打开输出URL（开始发送RTP流）
    ret = avio_open(&formatContext->pb, formatContext->url, AVIO_FLAG_WRITE);
    if (ret < 0) {
        fprintf(stderr, "Could not open output URL: %s\n", av_err2str(ret));
        return 1;
    }

    // 写入文件头部
    ret = avformat_write_header(formatContext, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output URL: %s\n", av_err2str(ret));
        return 1;
    }

    // 发送视频数据（这里简化为发送一帧白色图像）
    AVFrame* frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = codecContext->width;
    frame->height = codecContext->height;
    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate video frame data\n");
        return 1;
    }

    // 将画面填充为白色
    for (int y = 0; y < codecContext->height; y++) {
        for (int x = 0; x < codecContext->width; x++) {
            frame->data[0][y * frame->linesize[0] + x] = 255; // Y
            frame->data[1][y / 2 * frame->linesize[1] + x / 2] = 128; // U
            frame->data[2][y / 2 * frame->linesize[2] + x / 2] = 128; // V
        }
    }

    // 编码并发送帧
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    ret = avcodec_send_frame(codecContext, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return 1;
    }

    ret = avcodec_receive_packet(codecContext, &packet);
    if (ret < 0) {
        fprintf(stderr, "Error receiving encoded packet\n");
        return 1;
    }

    // 将包写入输出流
    ret = av_interleaved_write_frame(formatContext, &packet);
    if (ret < 0) {
        fprintf(stderr, "Error while writing video packet: %s\n", av_err2str(ret));
        return 1;
    }

    // 清理工作
    av_write_trailer(formatContext);
    avcodec_free_context(&codecContext);
    av_frame_free(&frame);
    avformat_free_context(formatContext);

    return 0;
}
