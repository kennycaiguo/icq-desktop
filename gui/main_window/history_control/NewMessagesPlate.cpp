#include "stdafx.h"
#include "NewMessagesPlate.h"
#include "../../utils/utils.h"


namespace Ui
{
	NewMessagesPlate::NewMessagesPlate(QWidget* parent)
		: QWidget(parent)
		, unreads_(0)
	{
		setStyleSheet(Utils::LoadStyle(":/main_window/history_control/history_control.qss", Utils::get_scale_coefficient(), true));
        if (this->objectName().isEmpty())
            this->setObjectName(QStringLiteral("new_messages_plate"));
        this->resize(796, 437);
        this->setProperty("NewMessages", QVariant(true));
        horizontal_layout_ = new QHBoxLayout(this);
        horizontal_layout_->setSpacing(0);
        horizontal_layout_->setObjectName(QStringLiteral("horizontalLayout"));
        horizontal_layout_->setContentsMargins(0, 0, 0, 0);
        horizontal_spacer_ = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        
        horizontal_layout_->addItem(horizontal_spacer_);
        
        widget_ = new QWidget(this);
        widget_->setObjectName(QStringLiteral("widget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget_->sizePolicy().hasHeightForWidth());
        widget_->setSizePolicy(sizePolicy);
        widget_->setMouseTracking(true);
        widget_->setProperty("NewMessagesWidget", QVariant(true));
        vertical_layout_ = new QVBoxLayout(widget_);
        vertical_layout_->setSpacing(0);
        vertical_layout_->setObjectName(QStringLiteral("verticalLayout"));
        vertical_layout_->setContentsMargins(0, 0, 0, 0);
        vertical_spacer_ = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        
        vertical_layout_->addItem(vertical_spacer_);
        
        message_ = new QLabel(widget_);
        message_->setObjectName(QStringLiteral("message"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(message_->sizePolicy().hasHeightForWidth());
        message_->setSizePolicy(sizePolicy1);
        message_->setMouseTracking(true);
        message_->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        message_->setWordWrap(false);
        message_->setProperty("NewMessagesLabel", QVariant(true));
        
        vertical_layout_->addWidget(message_);
        
        vertical_spacer_2_ = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        
        vertical_layout_->addItem(vertical_spacer_2_);
        
        
        horizontal_layout_->addWidget(widget_);
        
        horizontal_spacer_2_ = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        
        horizontal_layout_->addItem(horizontal_spacer_2_);
        
        message_->setText(QString());
        
        QMetaObject::connectSlotsByName(this);
        
        
		setMouseTracking(true);
		widget_->installEventFilter(this);
		widget_->setCursor(Qt::PointingHandCursor);
	}

	NewMessagesPlate::~NewMessagesPlate()
	{
	}

	bool NewMessagesPlate::eventFilter(QObject* obj, QEvent* event)
	{
		if (qobject_cast<QWidget*>(obj) == widget_)
		{
			if (event->type() == QEvent::Enter)
			{
				widget_->setProperty("NewMessagesWidget", false);
				widget_->setProperty("NewMessagesWidgetHover", true);
				widget_->setProperty("NewMessagesWidgetPressed", false);
				widget_->setStyle(QApplication::style());
				message_->setProperty("NewMessagesLabel", false);
				message_->setProperty("NewMessagesLabelHover", true);
				message_->setStyle(QApplication::style());
				return true;
			}
			else if (event->type() == QEvent::Leave)
			{
				widget_->setProperty("NewMessagesWidget", true);
				widget_->setProperty("NewMessagesWidgetHover", false);
				widget_->setProperty("NewMessagesWidgetPressed", false);
				widget_->setStyle(QApplication::style());
				message_->setProperty("NewMessagesLabel", true);
				message_->setProperty("NewMessagesLabelHover", false);
				message_->setStyle(QApplication::style());
				return true;
			}
			else if (event->type() == QEvent::MouseButtonPress)
			{
				widget_->setProperty("NewMessagesWidget", false);
				widget_->setProperty("NewMessagesWidgetHover", false);
				widget_->setProperty("NewMessagesWidgetPressed", true);
				widget_->setStyle(QApplication::style());
				message_->setProperty("NewMessagesLabel", false);
				message_->setProperty("NewMessagesLabelHover", true);
				message_->setStyle(QApplication::style());
				return true;
			}
			if (event->type() == QEvent::MouseButtonRelease)
			{
				widget_->setProperty("NewMessagesWidget", false);
				widget_->setProperty("NewMessagesWidgetHover", true);
				widget_->setProperty("NewMessagesWidgetPressed", false);
				widget_->setStyle(QApplication::style());
				message_->setProperty("NewMessagesLabel", false);
				message_->setProperty("NewMessagesLabelHover", true);
				message_->setStyle(QApplication::style());
				emit downPressed();
				return true;
			}
		}

		return QWidget::eventFilter(obj, event);
	}
	
	void NewMessagesPlate::setUnreadCount(int count)
	{
		unreads_ = count;
		QString message = QString("%1 " + Utils::GetTranslator()->getNumberString(count, QT_TRANSLATE_NOOP3("chat_page", "new message", "1"), QT_TRANSLATE_NOOP3("chat_page", "new messages", "2"),
			QT_TRANSLATE_NOOP3("chat_page", "new messages", "5"), QT_TRANSLATE_NOOP3("chat_page", "new messages", "21"))).arg(count);
		message_->setText(message);
		setMinimumHeight(message_->height());
	}

	void NewMessagesPlate::addUnread()
	{
		++unreads_;
		setUnreadCount(unreads_);
	}
	
	void NewMessagesPlate::setWidth(int width)
	{
		resize(width, height());
	}
}