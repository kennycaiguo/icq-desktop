#pragma once

#include "MessageContentWidget.h"

namespace Ui
{
    class TextEditEx;
}

namespace HistoryControl
{
    enum PlayingState
    {
        ptt_audio_ready = 0,
        ptt_audio_downloading,
        ptt_audio_playing,
        ptt_audio_pause,
        ptt_audio_done,
    };

    class PttAudioWidget : public MessageContentWidget
    {
        Q_OBJECT

    public:
        Q_PROPERTY(int Progress READ getProgress WRITE setProgress)
        Q_PROPERTY(int Angle READ getAngle WRITE setAngle)

        PttAudioWidget(QWidget *parent, const QString& contact, const bool isOutgoing, const QString& uri, int duration, qint64 id, qint64 prevId);

        virtual void initialize();
        virtual int32_t actualContentWidth();
        virtual bool isBlockElement() const;
        virtual bool canUnload() const;
        virtual void render(QPainter &p) override;
        virtual QString toLogString() const override;
        virtual QString toString() const override;

        virtual QPoint deliveryStatusOffsetHint(const int32_t statusLineWidth) const;

        void setProgress(int value);
        int getProgress() const;

        void setAngle(int value);
        int getAngle() const;

    public Q_SLOTS:
        void pttPlayed(qint64);

    protected:
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseReleaseEvent(QMouseEvent *);

    private:
        QBrush getBodyBrush(bool isOutgoing, bool isSelected);
        QBrush getProgressBrush(bool isOutgoing);
        QPainterPath getBodyPath(const QRect &rect, const int32_t borderRadius, const bool isOutgoing);
        void play();
        void pause();
        bool isMouseOnPlayButton(const QPoint& pos);
        bool isMouseOnTextButton(const QPoint& pos);
        void startProgressAnimation();
        void stopProgressAnimation();
        void pauseProgressAnimation();
        void startDownloadAnimation();
        void stopDownloadAnimation();
        void showText();
        void updateHeight();

    private Q_SLOTS:
        void messageDownloaded(qint64, QString, QString);
        void messageDownloadError(qint64, QString, qint32);
        void setFileInfo(qint64, QString, QString, QString, QString, QString, qint64, bool);
        void speechToText(qint64, int, QString);
        void pttFinished(int);
        void pttPaused(int);

    private:
        QPainterPath ProgressPath_;
        QString Uri_;
        QString DurationText_;
        QString Contact_;
        QString LocalPath_;
        PlayingState AudioState_;
        qint64 DownloadId_;
        qint64 MetaDownloadId_;
        qint64 FileSize_;
        qint64 Id_;
        qint64 PrevId_;
        qint64 TextSeq_;
        int Duration_;
        int PlayingId_;
        int ProgressValue_;
        int AngleValue_;
        bool IsOutgoing_;
        bool PlayPressed_;
        bool PlayHover_;
        bool TextPressed_;
        bool TextHover_;
        bool Played_;
        bool HaveText_;
        bool TextDownloading_;
        QPropertyAnimation* Progress_;
        QPropertyAnimation* Download_;
        QLabel* PttText_;
    };

}
