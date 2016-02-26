#pragma once

#include "../../types/message.h"

#include "HistoryControlPageItem.h"

namespace HistoryControl
{
	class MessageContentWidget;
}

namespace Themes
{
    enum class PixmapResourceId;
}

namespace Ui
{
	class message_item;
	class TextEditEx;
	class TextEmojiWidget;
	class PictureWidget;
	class ContextMenu;

	class MessageData
	{
	public:
		MessageData()
			: AvatarVisible_(false)
			, SenderVisible_(false)
			, IndentBefore_(false)
			, Sending_(false)
            , DeliveredToServer_(false)
            , DeliveredToClient_(false)
			, Id_(-1)
			, AvatarSize_(-1)
			, Time_(0)
		{
		}

		bool AvatarVisible_;
		bool SenderVisible_;
		bool IndentBefore_;
		bool Sending_;
		QDate Date_;
		bool Outgoing_;
		bool DeliveredToServer_;
        bool DeliveredToClient_;
		bool Chat_;
		qint64 Id_;
		QStringList NotificationsKeys_;
		QString AimId_;
		QString Sender_;
		QString Text_;
		QString StickerText_;
        QString SenderFriendly_;
		int AvatarSize_;
		qint32 Time_;
	};

	enum SelectDirection
	{
		NONE = 0,
		DOWN,
		UP,
	};

    class MessageItemLayout;

	class MessageItem : public HistoryControlPageItem
	{
        friend class MessageItemLayout;

		Q_OBJECT

	Q_SIGNALS:
		void copy(QString);
		void quote(QString);

	public:
        MessageItem();
		MessageItem(QWidget* parent);
		~MessageItem();

		virtual QString formatRecentsText() const override;

		void selectByPos(const QPoint& pos);
		QString selection(bool textonly = false);

		void setId(qint64 id, const QString& aimId);
		void setNotificationKeys(const QStringList& keys);
		const QStringList& getNotificationKeys();
		void waitForAvatar(bool wait);
		void setAvatarVisible(const bool);
		void setMessage(const QString& message);
        void setMchatSenderAimId(const QString& senderAimId);
        inline const QString& getMchatSenderAimId() const { return MessageSenderAimId_; };
		void setMchatSender(const QString& sender);
		void setOutgoing(const bool isOutgoing, const bool sending, const bool isDeliveredToServer, const bool isDeliveredToClient, const bool isMChat);
		void setTime(const int32_t time);
		void setContentWidget(::HistoryControl::MessageContentWidget *widget);
		void setStickerText(const QString& text);
		virtual void setTopMargin(const bool value) override;
		void setDate(const QDate& date);
		void replace();
		bool selected();
		QDate date() const;
		qint64 getId() const;
		bool isPersistent() const;
		QString toLogString() const;
        virtual void select();
        virtual void clearSelection();
        void manualUpdateGeometry(const int32_t widgetWidth);

		bool updateData(const std::shared_ptr<MessageData> &data);
		std::shared_ptr<MessageData> getData();

        void loadAvatar(const QString& sender, const QString& senderName, const int size);

        virtual QSize sizeHint() const override;

	private Q_SLOTS:
        void deliveredToClient(qint64);
		void deliveredToClient(QString);
		void deliveredToServer(QString);
		void avatarChanged(QString);
        void avatarClicked();
		void menu(QAction*);

	protected:
        virtual void leaveEvent(QEvent*) override;

        virtual void mouseMoveEvent(QMouseEvent*) override;

        virtual void mousePressEvent(QMouseEvent*) override;

        virtual void mouseReleaseEvent(QMouseEvent*) override;

        virtual void paintEvent(QPaintEvent*) override;

        virtual void resizeEvent(QResizeEvent*) override;

	private:
        void createMessageBody();

        void createSenderControl();

        void drawAvatar(QPainter &p);

        void drawDeliveryStatusAndTime(QPainter &p);

        void drawMessageBubble(QPainter &p);

        QRect evaluateAvatarRect() const;

        QRect evaluateContentHorGeometry(const int32_t contentWidth) const;

        QRect evaluateContentVertGeometry() const;

        int32_t evaluateContentWidth(const int32_t widgetWidth) const;

        int32_t evaluateDesiredContentHeight() const;

        int32_t evaluateLeftContentMargin() const;

        int32_t evaluateRightContentMargin() const;

        int32_t evaluateStatusLineWidth() const;

        int32_t evaluateTopContentMargin() const;

        const QRect& getAvatarRect() const;

        Themes::PixmapResourceId getDeliveryStatusResId() const;

        bool isAvatarVisible() const;

        bool isBlockItem() const;

        bool isOutgoing() const;

        bool isOverAvatar(const QPoint &p) const;

        bool isSending() const;

        void updateBubbleGeometry(const QRect &bubbleGeometry);

        void updateContentWidgetHorGeometry(const QRect &bubbleHorGeometry);

        void updateMessageBodyHorGeometry(const QRect &bubbleHorGeometry);

        void updateMessageBodyFullGeometry(const QRect &bubbleRect);

        void updateSenderGeometry();

		void connectDeliverySignals(const bool isConnected);
		bool isMessageBubbleVisible() const;

		TextEditEx *MessageBody_;
		TextEmojiWidget *Sender_;
        QString MessageSenderAimId_;
		::HistoryControl::MessageContentWidget *ContentWidget_;

		SelectDirection Direction_;
        QPoint PrevPos_;
		ContextMenu* Menu_;

		std::shared_ptr<MessageData> Data_;

        QPixmap Avatar_;

        QPainterPath Bubble_;

        mutable QRect AvatarRect_;

        QString TimeText_;

        QSize TimeTextSize_;

        bool ClickedOnAvatar_;

        MessageItemLayout *Layout_;
	};
}
