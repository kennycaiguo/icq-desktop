#pragma once

namespace HistoryControl
{

	class MessageContentWidget : public QWidget
	{
		Q_OBJECT

	Q_SIGNALS:
		void removeMe();

        void forcedLayoutUpdatedSignal();

    public:
        virtual QPoint deliveryStatusOffsetHint(const int32_t statusLineWidth) const;

        virtual bool isBlockElement() const = 0;

        virtual bool canUnload() const = 0;

        virtual void render(QPainter &p) = 0;

        virtual QString toLogString() const = 0;

        virtual QString toString() const = 0;

	public:
		MessageContentWidget(QWidget *parent, const bool isOutgoing);

		virtual ~MessageContentWidget();

        bool isSelected() const;

        void select(const bool value);

        virtual QString toRecentsString() const;

	protected:
		bool isOutgoing() const;

        void connectCoreSignal(const char *coreSignal, const char *ourSlot, const bool toConnect);

        int64_t getCurrentProcessId() const;

        bool isCurrentProcessId(const int64_t id) const;

        void resetCurrentProcessId();

        void setCurrentProcessId(const int64_t id);

		void setFixedSize(const QSize &size);

		void setFixedSize(const int32_t w, const int32_t h);

        virtual void initialize() = 0;

    protected:
        bool Selected_;

	private:
        int64_t CurrentProcessId_;

        bool Initialized_;

		const bool IsOutgoing_;

		void connectCoreSignal(const char *coreSignal, const char *ourSlot);

		void disconnectCoreSignal(const char *coreSignal, const char *ourSlot);

        virtual void paintEvent(QPaintEvent*) override final;

        virtual void showEvent(QShowEvent*) override final;

	};

}