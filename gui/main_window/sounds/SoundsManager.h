#pragma once

#include <AL/al.h>
#include <Al/alc.h>

class QSound;

namespace Ui
{
    struct PlayingData
    {
        PlayingData()
            : Source_(0)
            , Buffer_(0)
            , Id_(-1)
        {
        }

        void init();
        void setBuffer(const QByteArray& data, qint64 freq, qint64 fmt);
        void play();
        void pause();
        void stop();
        void clear();
        bool isEmpty() const;
        ALenum state() const;

        ALuint Source_;
        ALuint Buffer_;
        int Id_;
    };

	class SoundsManager : public QObject
	{
		Q_OBJECT
Q_SIGNALS:
        void pttPaused(int);
        void pttFinished(int);

	public:
		SoundsManager();
		~SoundsManager();

		void playIncomingMessage();
		void playOutgoingMessage();

        int playPtt(const QString& file, int id);
        void pausePtt(int id);

		void callInProgress(bool value);

	private Q_SLOTS:
		void timedOut();
        void checkPttState();
        void contactChanged(QString);

        void initOpenAl();
        void shutdownOpenAl();

	private:
		bool CallInProgress_;
		bool CanPlayIncoming_;

		QSound* IncomingMessageSound_;
		QSound* OutgoingMessageSound_;
		QTimer* Timer_;
        QTimer* PttTimer_;
        int PttId_;

        ALCdevice *AlAudioDevice_;
        ALCcontext *AlAudioContext_;
        PlayingData CurPlay_;
        PlayingData PrevPlay_;
        bool AlInited_;
	};

	SoundsManager* GetSoundsManager();
}