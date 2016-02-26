#pragma once

namespace core
{
    struct istream;
}

namespace Ui
{
    class LoadPixmapFromDataTask
        : public QObject
        , public QRunnable
    {
        Q_OBJECT

    Q_SIGNALS:
        void loadedSignal(qint64, QString, QPixmap);

    public:
        LoadPixmapFromDataTask(const qint64 seq, const QString &uri, core::istream *stream);

        virtual ~LoadPixmapFromDataTask();

        void run();

    private:
        const qint64 Seq_;

        const QString Uri_;

        core::istream *Stream_;

    };

}

