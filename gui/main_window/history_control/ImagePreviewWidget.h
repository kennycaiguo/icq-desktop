#pragma once

#include "PreviewContentWidget.h"

namespace HistoryControl
{

	class ImagePreviewWidget : public PreviewContentWidget
	{
		Q_OBJECT

	public:
		ImagePreviewWidget(QWidget *parent, const bool isOutgoing, const QString &uri, const QString &text, const bool previewsEnabled);

		virtual ~ImagePreviewWidget();

		virtual bool isBlockElement() const override;

		virtual bool canUnload() const override;

		virtual QString toLogString() const override;

		virtual QString toString() const override;

    protected:
        virtual void initialize() override;

        virtual bool isPreloaderVisible() const override;

        virtual void leaveEvent(QEvent *e) override;

        virtual void mouseMoveEvent(QMouseEvent *e) override;

        virtual void mousePressEvent(QMouseEvent *e) override;

        virtual void mouseReleaseEvent(QMouseEvent *e) override;

    private Q_SLOTS:
        void imageDownloaded(qint64, QString, QPixmap);

	private:
        enum class State;

        static bool isLeftButtonClick(QMouseEvent *e);

        void connectCoreSignals(const bool isConnected);

        bool isOverPicture(const QPoint &p) const;

        bool isPreviewVisible() const;

        State State_;

        const QString Uri_;

        bool LeftButtonPressed_;

	};

}