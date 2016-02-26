#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavfilter/avfilter.h>
};

namespace Ui
{
    class BaseMpegLoader 
    {
    public:

        BaseMpegLoader(const QString &file);
        ~BaseMpegLoader();

        bool open();
        qint64 duration();
        qint32 frequency();

    protected:

        qint32 Freq_;
        qint64 Len_;
        AVFormatContext* FmtContext_;
        AVCodec* Codec_;
        qint32 StreamId_;
        QString File_;
        bool Opened_;
    };

    class MpegLoader : public BaseMpegLoader 
    {
    public:

        MpegLoader(const QString &file);
        ~MpegLoader();

        bool open();
        qint32 format();
        int readMore(QByteArray &result, qint64 &samplesAdded);

    private:

        qint32 Fmt_;
        qint32 SrcRate_, DstRate_, MaxResampleSamples_, SampleSize_;
        uint8_t** OutSamplesData_;
        AVCodecContext* CodecContext_;
        AVPacket Avpkt_;
        AVSampleFormat InputFormat_;
        AVFrame* Frame_;
        SwrContext* SwrContext_;
    };
}