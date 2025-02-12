#pragma once


namespace Ui
{
	class new_messages_plate;

	class NewMessagesPlate : public QWidget
	{
		Q_OBJECT
	Q_SIGNALS:
		void downPressed();

	public:
		NewMessagesPlate(QWidget* parent);
		~NewMessagesPlate();

		void setWidth(int width);
		void setUnreadCount(int count);
		void addUnread();

	protected:
		bool eventFilter(QObject* obj, QEvent* event);

	private:
		int unreads_;
        QHBoxLayout *horizontal_layout_;
        QSpacerItem *horizontal_spacer_;
        QWidget *widget_;
        QVBoxLayout *vertical_layout_;
        QSpacerItem *vertical_spacer_;
        QLabel *message_;
        QSpacerItem *vertical_spacer_2_;
        QSpacerItem *horizontal_spacer_2_;
	};
}