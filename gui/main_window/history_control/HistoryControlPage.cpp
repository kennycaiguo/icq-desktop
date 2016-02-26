#include "stdafx.h"

#include "../../../corelib/enumerations.h"

#include "HistoryControlPage.h"
#include "ServiceMessageItem.h"
#include "ChatEventItem.h"
#include "VoipEventItem.h"
#include "MessagesModel.h"
#include "MessageItem.h"
#include "ServiceMessageItem.h"
#include "FileSharingWidget.h"
#include "NewMessagesPlate.h"
#include "MessagesScrollArea.h"

#include "../contact_list/ContactListModel.h"
#include "../contact_list/ChatMembersModel.h"
#include "../contact_list/RecentsModel.h"
#include "../../core_dispatcher.h"

#include "../../utils/Text2DocConverter.h"
#include "../../utils/utils.h"
#include "../../utils/gui_coll_helper.h"
#include "../../utils/log/log.h"
#include "../../utils/InterConnector.h"
#include "../../utils/profiling/auto_stop_watch.h"
#include "auth_widget/AuthWidget.h"
#include "../../controls/SemitransparentWindow.h"
#include "../../controls/ContextMenu.h"
#include "../../gui_settings.h"
#include "../contact_list/SelectionContactsForGroupChat.h"
#include "../contact_list/ContactList.h"
#include "../contact_list/Common.h"
#include "../../controls/LabelEx.h"
#include "../../controls/TextEmojiWidget.h"
#include "../MainPage.h"
#include "../ContactDialog.h"
#include "../smiles_menu/SmilesMenu.h"

namespace
{
	bool isPersistentWidget(QWidget *w);
	QMap<QString, QVariant> makeData(const QString& command, const QString& aimid = QString());
}

namespace Ui
{
	MessagesWidgetEventFilter::MessagesWidgetEventFilter(
        QWidget* top, QWidget* buttonsWidget, const QString& contactName, QTextBrowser* contactNameWidget,
		MessagesScrollArea *scrollArea, QWidget* firstOverlay, QWidget* secondOverlay,
        NewMessagesPlate* newMessaesPlate, HistoryControlPage* dialog
    )
		: QObject(dialog)
		, TopWidget_(top)
		, ScrollArea_(scrollArea)
		, FirstOverlay_(firstOverlay)
		, SecondOverlay_(secondOverlay)
		, Dialog_(dialog)
		, Width_(0)
		, NewPlateShowed_(false)
		, ContactName_(contactName)
		, ContactNameWidget_((QTextBrowser *)contactNameWidget)
		, ButtonsWidget_(buttonsWidget)
		, NewMessagesPlate_(newMessaesPlate)
		, ScrollDirectionDown_(false)
		, Timer_(new QTimer(this))
	{
		assert(ContactNameWidget_);
		assert(!ContactName_.isEmpty());
        assert(ScrollArea_);
        assert(MessagesWidget_);

		NewMessagesPlate_->hide();
		Timer_->setSingleShot(false);
		Timer_->setInterval(100);
	}

    void MessagesWidgetEventFilter::resetNewPlate()
	{
		NewPlateShowed_ = false;
	}

    QString MessagesWidgetEventFilter::getContactName() const
    {
        return ContactName_;
    }

	bool MessagesWidgetEventFilter::eventFilter(QObject* obj, QEvent* event)
	{
		if (event->type() == QEvent::Resize)
		{
            const auto rect = qobject_cast<QWidget*>(obj)->rect();

            TopWidget_->setFixedWidth(rect.width());
			TopWidget_->move(rect.x(), rect.y());

			FirstOverlay_->setMinimumWidth(rect.width());
			FirstOverlay_->setMaximumWidth(rect.width());
			FirstOverlay_->move(rect.x(), rect.y() + TopWidget_->height());
			SecondOverlay_->setMinimumWidth(rect.width());
			SecondOverlay_->setMaximumWidth(rect.width());
			SecondOverlay_->move(rect.x(), rect.y() + TopWidget_->height() + FirstOverlay_->height() * 0.7);
            ScrollArea_->setGeometry(
                rect.x(),
                rect.y() + TopWidget_->height(),
                rect.width(),
                rect.height() - TopWidget_->height()
            );
			FirstOverlay_->stackUnder(SecondOverlay_);
			TopWidget_->stackUnder(FirstOverlay_);
			ScrollArea_->stackUnder(TopWidget_);
			FirstOverlay_->hide();
			qobject_cast<Ui::ServiceMessageItem*>(SecondOverlay_)->setNew();
			SecondOverlay_->hide();
			NewMessagesPlate_->move(rect.x(), rect.height() - NewMessagesPlate_->height());
			NewMessagesPlate_->setWidth(rect.width());
			if (Width_ != rect.width())
			{
				Logic::GetMessagesModel()->setItemWidth(rect.width());
			}

			Width_ = rect.width();

			ResetContactName(ContactName_);
		}
		else if (event->type() == QEvent::Paint)
		{
			QDate date;
			auto newFound = false;
			auto dateVisible = true;
			qint64 firstVisibleId = -1;

            ScrollArea_->enumerateWidgets(
                [this, &firstVisibleId, &newFound, &date, &dateVisible]
                (QWidget *widget, const bool isVisible)
                {
                    if (widget->visibleRegion().isEmpty() || !isVisible)
                    {
                        return true;
                    }

                    if (auto msgItem = qobject_cast<Ui::MessageItem*>(widget))
                    {
                        if (firstVisibleId == -1)
                        {
                            date = msgItem->date();
                            firstVisibleId = msgItem->getId();
                        }

                        return true;
                    }

                    if (auto serviceItem = qobject_cast<Ui::ServiceMessageItem*>(widget))
                    {
                        if (serviceItem->isNew())
                        {
                            newFound = true;
                            NewPlateShowed_ = true;
                        }
                        else
                        {
                            dateVisible = false;
                        }
                    }

                    return true;
                },
                false
            );

			if (!newFound && NewPlateShowed_)
			{
				Dialog_->newPlateShowed();
				NewPlateShowed_ = false;
			}

			bool visible = date.isValid();

			if (date != Date_)
			{
				Date_ = date;
				qobject_cast<Ui::ServiceMessageItem*>(FirstOverlay_)->setDate(Date_);
				FirstOverlay_->adjustSize();
			}

            const auto isFirstOverlayVisible = (dateVisible && visible);
 			FirstOverlay_->setAttribute(Qt::WA_WState_Hidden, !isFirstOverlayVisible);
			FirstOverlay_->setAttribute(Qt::WA_WState_Visible, isFirstOverlayVisible);

			qint64 newPlateId = Dialog_->getNewPlateId();
			bool newPlateOverlay = newPlateId != -1 && newPlateId < firstVisibleId && !newFound && !NewPlateShowed_;

            if (newPlateOverlay && visible)
            {
                if (!SecondOverlay_->testAttribute(Qt::WA_WState_Visible))
                    SecondOverlay_->show();
            }
            else
            {
                if (!SecondOverlay_->testAttribute(Qt::WA_WState_Hidden))
                    SecondOverlay_->hide();
            }
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			Timer_->stop();
		}
		else if (event->type() == QEvent::KeyPress)
		{
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->matches(QKeySequence::Copy))
			{
				const auto result = ScrollArea_->getSelectedText();

				if (!result.isEmpty())
                {
					QApplication::clipboard()->setText(result);
                }
			}
		}

		return QObject::eventFilter(obj, event);
	}

	void MessagesWidgetEventFilter::ResetContactName(QString _contact_name)
	{
		if (!ContactNameWidget_)
		{
			return;
		}

        ContactName_ = _contact_name;

		auto contactNameMaxWidth = TopWidget_->contentsRect().width();
		contactNameMaxWidth -= ButtonsWidget_->width();
		ContactNameWidget_->setFixedWidth(contactNameMaxWidth);

		const auto contactNameInnerWidth = ContactNameWidget_->contentsRect().width();

		QFontMetrics m(ContactNameWidget_->font());
		const auto elidedString = m.elidedText(ContactName_, Qt::ElideRight, contactNameInnerWidth);
		auto &doc = *ContactNameWidget_->document();
		doc.clear();

		QTextCursor textCursor = ContactNameWidget_->textCursor();
		Logic::Text2Doc(elidedString, textCursor, Logic::Text2DocHtmlMode::Pass, false);
		Logic::FormatDocument(doc, m.height());
	}

    class HistoryControlPage::PositionInfo
	{
	public:
		PositionInfo(const Logic::MessageKey& key, QWidget *widget);

		bool operator<(const PositionInfo& rhs) const;

		const QWidget* getWidget() const;

        qint32 getPosition() const;

		void setPosition(const qint32 pos);

		Logic::MessageKey toMessageKey() const;

        void setMessageKey(const Logic::MessageKey& key);

	private:
		Logic::MessageKey Key_;

		qint32 Position_;

#if defined(DEBUG)
		QWidget *Widget_;
#endif

	};

	HistoryControlPage::PositionInfo::PositionInfo(const Logic::MessageKey& key, QWidget *widget)
		: Key_(key)
		, Position_(0)
	{
#if defined(DEBUG)
		Widget_ = widget;
		assert(Widget_);
#else
		widget;
#endif
	}

    bool HistoryControlPage::PositionInfo::operator<(const PositionInfo& rhs) const
    {
        return Key_ < rhs.Key_;
    }

	qint32 HistoryControlPage::PositionInfo::getPosition() const
	{
		assert(Position_ >= 0);

		return Position_;
	}

	const QWidget* HistoryControlPage::PositionInfo::getWidget() const
	{
#ifdef DEBUG
		return Widget_;
#else
		return nullptr;
#endif
	}

	void HistoryControlPage::PositionInfo::setPosition(const qint32 pos)
	{
		assert(pos >= 0);
		Position_ = pos;
	}

    void HistoryControlPage::PositionInfo::setMessageKey(const Logic::MessageKey& key)
    {
        Key_ = key;
    }

	Logic::MessageKey HistoryControlPage::PositionInfo::toMessageKey() const
	{
		return Key_;
	}

    enum class HistoryControlPage::State
    {
        Min,

        Idle,
        Fetching,
        Inserting,

        Max
    };

    QTextStream& operator<<(QTextStream &oss, const HistoryControlPage::State arg)
    {
        switch (arg)
        {
            case HistoryControlPage::State::Idle: oss << "IDLE"; break;
            case HistoryControlPage::State::Fetching: oss << "FETCHING"; break;
            case HistoryControlPage::State::Inserting: oss << "INSERTING"; break;

            default:
                assert(!"unexpected state value");
                break;
        }

        return oss;
    }

	HistoryControlPage::HistoryControlPage(QWidget* parent, QString aimid)
		: QWidget(parent)
		, aimId_(aimid)
		, messages_overlay_first_(new ServiceMessageItem(this, true))
		, messages_overlay_second_(new ServiceMessageItem(this, true))
        , messages_area_(new MessagesScrollArea(this, TypingWidget_))
        , TypingWidget_(new QWidget(this))
		, new_plate_position_(-1)
		, new_messages_plate_(new NewMessagesPlate(this))
		, chat_info_sequence_(-1)
		, auth_widget_(nullptr)
		, next_local_position_(0)
		, menu_(new ContextMenu(this))
        , contact_status_(new LabelEx(this))
        , chat_members_model_(NULL)
        , edit_members_button_(new QPushButton(this))
        , is_chat_member_(false)
        , is_contact_status_clickable_(false)
        , is_public_chat_(false)
        , state_(State::Idle)
        , is_messages_request_postponed_(false)
	{
        if (this->objectName().isEmpty())
            this->setObjectName(QStringLiteral("history_control_page"));
        this->resize(595, 422);

		setStyleSheet(Utils::LoadStyle(":/main_window/history_control/history_control.qss", Utils::get_scale_coefficient(), true));
        top_widget_ = new QWidget(this);
        top_widget_->setObjectName(QStringLiteral("top_widget"));
        top_widget_->setGeometry(QRect(9, 9, 581, 41));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(top_widget_->sizePolicy().hasHeightForWidth());
        top_widget_->setSizePolicy(sizePolicy);
        top_widget_->setProperty("ContactPageTopWidget", QVariant(true));
        horizontal_layout_ = new QHBoxLayout(top_widget_);
        horizontal_layout_->setSpacing(0);
        horizontal_layout_->setObjectName(QStringLiteral("horizontalLayout"));
        horizontal_layout_->setContentsMargins(0, 0, 0, 0);
        contact_widget_ = new QWidget(top_widget_);
        contact_widget_->setObjectName(QStringLiteral("contact_widget"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(contact_widget_->sizePolicy().hasHeightForWidth());
        contact_widget_->setSizePolicy(sizePolicy1);
        vertical_layout_ = new QVBoxLayout(contact_widget_);
        vertical_layout_->setSpacing(0);
        vertical_layout_->setObjectName(QStringLiteral("verticalLayout_2"));
        vertical_layout_->setSizeConstraint(QLayout::SetDefaultConstraint);
        vertical_layout_->setContentsMargins(0, 0, 0, 0);
        contact_name_ = new QTextBrowser(contact_widget_);
        contact_name_->setObjectName(QStringLiteral("contact_name"));
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(contact_name_->sizePolicy().hasHeightForWidth());
        contact_name_->setSizePolicy(sizePolicy2);
        contact_name_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        contact_name_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        contact_name_->setProperty("ContactPageName", QVariant(true));
        vertical_layout_->addWidget(contact_name_);
        horizontal_layout_2_ = new QHBoxLayout();
        horizontal_layout_2_->setSpacing(0);
        horizontal_layout_2_->setObjectName(QStringLiteral("horizontalLayout1"));
        horizontal_layout_2_->setProperty("layoutChatInfo", QVariant(true));
        horizontal_layout_2_->setContentsMargins(0, 0, 0, 0);
        contact_status_layout_ = new QHBoxLayout();
        contact_status_widget_ = new QWidget(this);
        contact_status_layout_->setObjectName(QStringLiteral("contact_status_layout"));

        horizontalSpacer_contact_status_ = new QSpacerItem(Utils::scale_value(15), Utils::scale_value(40), QSizePolicy::Fixed, QSizePolicy::Fixed);
        horizontal_layout_2_->addItem(horizontalSpacer_contact_status_);
        horizontal_layout_2_->addWidget(contact_status_widget_);
        edit_members_button_->setObjectName(QStringLiteral("edit_members_button"));
        edit_members_button_->setProperty("EditMembersButton", QVariant(true));
        Testing::setAccessibleName(edit_members_button_, "ShowChatMembers");
        horizontalSpacer_2_ = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
        horizontal_layout_2_->addItem(horizontalSpacer_2_);

        vertical_layout_->addLayout(horizontal_layout_2_);
        vertical_spacer_ = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        vertical_layout_->addItem(vertical_spacer_);
        horizontal_layout_->addWidget(contact_widget_);
        buttons_widget_ = new QWidget(top_widget_);
        buttons_widget_->setObjectName(QStringLiteral("buttons_widget"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(buttons_widget_->sizePolicy().hasHeightForWidth());
        buttons_widget_->setSizePolicy(sizePolicy3);
        grid_layout_ = new QGridLayout(buttons_widget_);
        grid_layout_->setSpacing(0);
        grid_layout_->setObjectName(QStringLiteral("gridLayout"));
        grid_layout_->setContentsMargins(0, 0, 0, 0);
        call_button_ = new QPushButton(buttons_widget_);
        call_button_->setObjectName(QStringLiteral("call_button"));
        call_button_->setProperty("CallButton", QVariant(true));
        grid_layout_->addWidget(call_button_, 0, 0, 1, 1, Qt::AlignRight);
        video_call_button_ = new QPushButton(buttons_widget_);
        video_call_button_->setObjectName(QStringLiteral("video_call_button"));
        video_call_button_->setProperty("VideoCallButton", QVariant(true));
        grid_layout_->addWidget(video_call_button_, 0, 1, 1, 1, Qt::AlignRight);
		add_member_button_ = new QPushButton(buttons_widget_);
		add_member_button_->setObjectName(QStringLiteral("add_member_button"));
		add_member_button_->setProperty("AddMemberButton", QVariant(true));
        Testing::setAccessibleName(add_member_button_, "AddContactToChat");
		grid_layout_->addWidget(add_member_button_, 0, 2, 1, 1, Qt::AlignRight);
		more_button_ = new QPushButton(buttons_widget_);
        more_button_->setObjectName(QStringLiteral("more_button"));
        more_button_->setProperty("OptionButton", QVariant(true));
        Testing::setAccessibleName(more_button_, "ShowChatMenu");
        grid_layout_->addWidget(more_button_, 0, 3, 1, 1, Qt::AlignRight);
        vertical_spacer_2_ = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        grid_layout_->addItem(vertical_spacer_2_, 1, 1, 1, 1);
        horizontal_layout_->addWidget(buttons_widget_, 0, Qt::AlignRight);
        edit_members_button_->setText(QString());
		add_member_button_->setText(QString());
        call_button_->setText(QString());
        video_call_button_->setText(QString());
        more_button_->setText(QString());
        Q_UNUSED(this);
        QMetaObject::connectSlotsByName(this);

		messages_overlay_first_->setAttribute(Qt::WA_TransparentForMouseEvents);
		event_filter_ = new MessagesWidgetEventFilter(
			top_widget_,
			buttons_widget_,
			Logic::GetContactListModel()->selectedContactName(),
			contact_name_,
            messages_area_,
			messages_overlay_first_,
			messages_overlay_second_,
			new_messages_plate_,
			this);
		installEventFilter(event_filter_);

		TypingWidget_->setProperty("TypingWidget", true);
        {
            auto twl = new QHBoxLayout(TypingWidget_);
            twl->setContentsMargins(Utils::scale_value(62), 0, 0, 0);
            twl->setSpacing(Utils::scale_value(7));
            twl->setAlignment(Qt::AlignLeft);
            {
                typingWidgets_.twa = new QLabel(TypingWidget_);
                typingWidgets_.twa->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
                typingWidgets_.twa->setContentsMargins(0, Utils::scale_value(11), 0, 0);

                typingWidgets_.twm = new QMovie(":/resources/typing_animation200.gif");
                typingWidgets_.twm->setScaledSize(QSize(Utils::scale_value(16), Utils::scale_value(8)));
                typingWidgets_.twa->setMovie(typingWidgets_.twm);
                typingWidgets_.twa->setVisible(false);
                twl->addWidget(typingWidgets_.twa);

                typingWidgets_.twt = new TextEmojiWidget(TypingWidget_, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(12), QColor("#57544c"), Utils::scale_value(20));
                typingWidgets_.twt->setSizePolicy(QSizePolicy::Policy::Preferred, typingWidgets_.twt->sizePolicy().verticalPolicy());
                typingWidgets_.twt->setText(" ");
                typingWidgets_.twt->setVisible(false);

                twl->addWidget(typingWidgets_.twt);

                connect(GetDispatcher(), SIGNAL(typing(QString, QVector< QString >)), this, SLOT(typing(QString, QVector< QString >)));
                connect(GetDispatcher(), SIGNAL(stopTyping(QString, QVector< QString >, int)), this, SLOT(stopTyping(QString, QVector< QString >, int)));
            }
        }
        TypingWidget_->show();

		connect(add_member_button_, SIGNAL(clicked()), this, SLOT(add_member()), Qt::QueuedConnection);
		connect(video_call_button_, SIGNAL(clicked()), this, SLOT(callVideoButtonClicked()), Qt::QueuedConnection);
        connect(call_button_, SIGNAL(clicked()), this, SLOT(callAudioButtonClicked()), Qt::QueuedConnection);
        connect(more_button_, SIGNAL(clicked()), this, SLOT(moreButtonClicked()), Qt::QueuedConnection);

		more_button_->setFocusPolicy(Qt::NoFocus);
		more_button_->setCursor(Qt::PointingHandCursor);
		video_call_button_->setFocusPolicy(Qt::NoFocus);
		video_call_button_->setCursor(Qt::PointingHandCursor);
		call_button_->setFocusPolicy(Qt::NoFocus);
		call_button_->setCursor(Qt::PointingHandCursor);
		add_member_button_->setFocusPolicy(Qt::NoFocus);
		add_member_button_->setCursor(Qt::PointingHandCursor);

        messages_area_->setFocusPolicy(Qt::StrongFocus);

        QString edit_members_button_style = QString("") +
            "   border-image: url(:/resources/widgets/content_dropdown_black_open_100.png);"+
            "   min-width: 12dip;"+
            "   max-width: 12dip;"+
            "   min-height: 8dip;"+
            "   max-height: 8dip;"+
            "   margin-left: 2dip;"+
            "   margin-right: 6dip;"+
            "   margin-top: 9dip;"+
            "   margin-bottom: 8dip;"+
            "   background-color : transparent;";
        edit_members_button_->setStyleSheet(Utils::ScaleStyle(edit_members_button_style, Utils::get_scale_coefficient()));

        QString contact_status_style = "font-size: 15dip; color: #696969; background-color: transparent;";
        contact_status_->setStyleSheet(Utils::ScaleStyle(contact_status_style, Utils::get_scale_coefficient()));
        contact_status_layout_->addWidget(contact_status_);
        contact_status_layout_->setMargin(Utils::scale_value(5));
        contact_status_layout_->setSpacing(Utils::scale_value(5));

        contact_status_widget_->setLayout(contact_status_layout_);
        contact_status_layout_->addWidget(edit_members_button_);

#ifndef STRIP_VOIP
        if (Logic::GetContactListModel()->getContactItem(aimid)->is_chat())
        {
#endif //STRIP_VOIP
            call_button_->hide();
            video_call_button_->hide();
#ifndef STRIP_VOIP
        }
        else
        {
			add_member_button_->hide();
			edit_members_button_->hide();
        }
#endif //STRIP_VOIP

#ifdef STRIP_VOIP
        // TODO: Should remove the lines below when STRIP_VOIP is removed.
        if (!Logic::GetContactListModel()->getContactItem(aimid)->is_chat())
        {
            edit_members_button_->hide();
            add_member_button_->hide();
        }
#endif //STRIP_VOIP

        menu_->invertRight(true);
        menu_->setIndent(Utils::scale_value(24));
        updateMenu(aimId_);

        SetContactStatusClickable(false);
        connect(Logic::GetContactListModel(), SIGNAL(contactChanged(QString)), this, SLOT(updateMenu(QString)), Qt::QueuedConnection);
		connect(menu_, SIGNAL(triggered(QAction*)), this, SLOT(popup_menu(QAction*)), Qt::QueuedConnection);

		connect(Logic::GetMessagesModel(), SIGNAL(ready(QString)), this, SLOT(sourceReady(QString)), Qt::QueuedConnection);

		connect(Logic::GetMessagesModel(), SIGNAL(updated(QList<Logic::MessageKey>, QString, unsigned)), this, SLOT(updated(QList<Logic::MessageKey>, QString, unsigned)), Qt::QueuedConnection);
		connect(Logic::GetMessagesModel(), SIGNAL(deleted(QList<Logic::MessageKey>, QString)), this, SLOT(deleted(QList<Logic::MessageKey>, QString)), Qt::QueuedConnection);

		connect(Logic::GetContactListModel(), SIGNAL(contactChanged(QString)), this, SLOT(contactChanged(QString)), Qt::QueuedConnection);

		connect(Logic::GetMessagesModel(), SIGNAL(messageIdFetched(QString, Logic::MessageKey)), this, SLOT(messageKeyUpdated(QString, Logic::MessageKey)), Qt::QueuedConnection);

		connect(Logic::GetRecentsModel(), SIGNAL(readStateChanged(QString)), this, SLOT(update(QString)), Qt::QueuedConnection);

		auto success = connect(
            this, &HistoryControlPage::requestMoreMessagesSignal,
            this, &HistoryControlPage::requestMoreMessagesSlot,
            Qt::QueuedConnection
        );
        assert(success);

        #pragma message(__TODOA__ "gonna investigate this")
		//connect(messages_scrollbar_, SIGNAL(autoScroll(bool)), this, SLOT(autoScroll(bool)), Qt::QueuedConnection);
        success = QObject::connect(
            messages_area_, &MessagesScrollArea::fetchRequestedEvent,
            this, &HistoryControlPage::onReachedFetchingDistance
        );
        assert(success);

        success = QObject::connect(messages_area_, &MessagesScrollArea::needCleanup, this, &HistoryControlPage::needCleanup);
        assert(success);

        success = QObject::connect(messages_area_, &MessagesScrollArea::scrollMovedToBottom, this, &HistoryControlPage::scrollMovedToBottom);
        assert(success);

		connect(new_messages_plate_, SIGNAL(downPressed()), this, SLOT(downPressed()), Qt::QueuedConnection);

		success = connect(this, SIGNAL(insertNextMessageSignal()), this, SLOT(insertNextMessageSlot()), Qt::QueuedConnection);
        assert(success);

		connect(this, SIGNAL(needRemove(Logic::MessageKey)), this, SLOT(removeWidget(Logic::MessageKey)), Qt::QueuedConnection);

		connect(&Utils::InterConnector::instance(), SIGNAL(profileSettingsUnknownAdd(QString)), this, SLOT(auth_add_contact(QString)), Qt::QueuedConnection);
		connect(&Utils::InterConnector::instance(), SIGNAL(profileSettingsUnknownSpam(QString)), this, SLOT(auth_spam_contact(QString)), Qt::QueuedConnection);
		connect(&Utils::InterConnector::instance(), SIGNAL(profileSettingsUnknownIgnore(QString)), this, SLOT(auth_ignore_contact(QString)), Qt::QueuedConnection);
    }

    void HistoryControlPage::typing(QString aimId, QVector< QString > chattersAimIds)
    {
        if (aimId != aimId_)
            return;
        for (auto chatter: chattersAimIds)
            typingChattersAimIds_.insert(chatter);
        updateTypingWidgets();
    }

    void HistoryControlPage::stopTyping(QString aimId, QVector< QString > chattersAimIds, int)
    {
        if (aimId != aimId_)
            return;
        for (auto chatter: chattersAimIds)
            typingChattersAimIds_.remove(chatter);
        if (typingChattersAimIds_.empty())
            hideTypingWidgets();
        else
            updateTypingWidgets();
    }

    void HistoryControlPage::updateTypingWidgets()
    {
        if (!typingChattersAimIds_.empty() && typingWidgets_.twa && typingWidgets_.twm && typingWidgets_.twt)
        {
            QString named;
            if (typingChattersAimIds_.size() > 1 || (typingChattersAimIds_.size() == 1 && typingChattersAimIds_.toList().at(0) != aimId()))
            {
                for (auto chatter: typingChattersAimIds_)
                {
                    if (named.length())
                        named += ", ";
                    auto contact = Logic::GetContactListModel()->getContactItem(chatter);
                    if (contact)
                    {
                        named += contact->Get()->GetDisplayName();
                    }
                    else
                    {
                        named += chatter;
                    }
                }
            }
            typingWidgets_.twa->setVisible(true);
            typingWidgets_.twm->start();
            typingWidgets_.twt->setVisible(true);
            if (named.length() && typingChattersAimIds_.size() == 1)
                typingWidgets_.twt->setText(named + " " + QT_TRANSLATE_NOOP("typing_widget", "is typing"));
            else if (named.length() && typingChattersAimIds_.size() > 1)
                typingWidgets_.twt->setText(named + " " + QT_TRANSLATE_NOOP("typing_widget", "are typing"));
            else
                typingWidgets_.twt->setText(QT_TRANSLATE_NOOP("typing_widget", "is typing"));
        }
    }
    void HistoryControlPage::hideTypingWidgets()
    {
        if (typingWidgets_.twa && typingWidgets_.twm && typingWidgets_.twt)
        {
            typingWidgets_.twa->setVisible(false);
            typingWidgets_.twm->stop();
            typingWidgets_.twt->setVisible(false);
            typingWidgets_.twt->setText("");
        }
    }

	HistoryControlPage::~HistoryControlPage()
	{
        typingWidgets_.twa = nullptr;
        typingWidgets_.twm = nullptr;
        typingWidgets_.twt = nullptr;

        delete chat_members_model_;
	}

    void HistoryControlPage::appendAuthControlIfNeed()
    {
        auto contact_item = Logic::GetContactListModel()->getContactItem(aimId_);
        if (contact_item && contact_item->is_chat())
            return;

        if (auth_widget_)
            return;

        if (!contact_item || contact_item->is_not_auth())
        {
            auth_widget_ = new AuthWidget(messages_area_, aimId_);
            messages_area_->insertWidget(0, auth_widget_);

            connect(auth_widget_, SIGNAL(add_contact(QString)), this, SLOT(auth_add_contact(QString)));
            connect(auth_widget_, SIGNAL(spam_contact(QString)), this, SLOT(auth_spam_contact(QString)));
            connect(auth_widget_, SIGNAL(delete_contact(QString)), this, SLOT(auth_delete_contact(QString)));

            connect(Logic::GetContactListModel(), SIGNAL(contact_added(QString, bool)), this, SLOT(contact_authorized(QString, bool)));
        }
    }

	bool HistoryControlPage::isScrolling() const
	{
        return !messages_area_->isScrollAtBottom();
	}

	HistoryControlPage::PositionInfoListIter HistoryControlPage::getPositionInfoByKey(const Logic::MessageKey& key)
	{
        return std::find_if(
            position_list_.begin(),
            position_list_.end(),
            [key](const PositionInfoSptr &info)
            {
                assert(info);

                return (info->toMessageKey() == key);
            }
        );
	}

	QWidget* HistoryControlPage::getWidgetByKey(const Logic::MessageKey& key)
	{
        auto positionInfoIter = getPositionInfoByKey(key);
        if (positionInfoIter == position_list_.end())
        {
            return nullptr;
        }

        const auto &positionInfo = *positionInfoIter;

 		return messages_area_->getItemByPos(positionInfo->getPosition());
	}

	HistoryControlPage::WidgetRemovalResult HistoryControlPage::removeExistingWidgetByKey(const Logic::MessageKey& key)
	{
		auto positionInfoIter = getPositionInfoByKey(key);
		if (positionInfoIter == position_list_.end())
		{
			return WidgetRemovalResult::NotFound;
		}

		const auto positionInfo = *positionInfoIter;

		const auto position = positionInfo->getPosition();
		assert(position >= 0);

		auto widget = messages_area_->getItemByPos(position);
		if (!widget)
		{
			assert(!"no widget at the position");
			return WidgetRemovalResult::NotFound;
		}

		if (isPersistentWidget(widget))
		{
			return WidgetRemovalResult::PersistentWidget;
		}

		messages_area_->removeWidget(widget);
		widget->deleteLater();

		position_list_.erase(positionInfoIter);

		return WidgetRemovalResult::Removed;
	}

	void HistoryControlPage::recalculateWidgetsPositions()
	{
		auto pos = (qint32)(position_list_.size() - 1);
		std::for_each(
			position_list_.begin(),
			position_list_.end(),
			[&pos](PositionInfoSptr &info)
		    {
			    info->setPosition(pos);
			    --pos;
		    }
		);
	}

	const Ui::MessageItem* HistoryControlPage::getMessageItemAt(const qint32 pos) const
	{
		assert(pos > 0);
		/*assert(pos < messages_area_->getMessagesLayout()->count());

		const auto layoutItem = messages_area_->getMessagesLayout()->itemAt(pos);
		assert(layoutItem);

		const auto widget = layoutItem->widget();
		assert(widget);

		return qobject_cast<Ui::MessageItem*>(widget);*/

        return nullptr;
	}

	bool HistoryControlPage::hasMessageItemAt(const qint32 pos) const
	{
		assert(pos > 0);

        return false;

		//return (pos < messages_area_->getMessagesLayout()->count());
	}

	void HistoryControlPage::contact_authorized(QString _aimid, bool _res)
	{
		if (_res)
		{
            if (aimId_ == _aimid)
            {
                messages_area_->removeWidget(auth_widget_);

                auth_widget_->deleteLater();
                auth_widget_ = nullptr;
            }
		}
	}

	void HistoryControlPage::auth_add_contact(QString _aimid)
	{
		Logic::GetContactListModel()->add_contact_to_contact_list(_aimid);
	}

	void HistoryControlPage::auth_spam_contact(QString _aimid)
	{
		Logic::GetContactListModel()->block_spam_contact(_aimid);
	}

	void HistoryControlPage::auth_delete_contact(QString _aimid)
	{
		Logic::GetContactListModel()->remove_contact_from_contact_list(_aimid);

        Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
        collection.set_value_as_qstring("contact", _aimid);
        Ui::GetDispatcher()->post_message_to_core("dialogs/hide", collection.get());
	}

    void HistoryControlPage::auth_ignore_contact(QString _aimid)
    {
        Logic::GetContactListModel()->ignore_contact(_aimid, true);
        Logic::GetContactListModel()->remove_contact_from_contact_list(_aimid);
    }

	void HistoryControlPage::newPlateShowed()
	{
		qint64 oldNew = new_plate_position_;
		new_plate_position_ = -1;
		Logic::GetMessagesModel()->updateNew(aimId_, oldNew, true);
	}

	void HistoryControlPage::update(QString aimId)
	{
        if (aimId != aimId_)
            return;

		if (!isVisible())
		{
			Logic::GetMessagesModel()->updateNew(aimId_, new_plate_position_);

			Data::DlgState state = Logic::GetRecentsModel()->getDlgState(aimId_);
			new_plate_position_ = state.LastMsgId_ == state.YoursLastRead_ ? -1 : state.YoursLastRead_;
		}
	}

    void HistoryControlPage::copy(QString _text)
    {
        auto selection_text = messages_area_->getSelectedText();

        if (!selection_text.isEmpty())
            QApplication::clipboard()->setText(selection_text);
        else
            QApplication::clipboard()->setText(_text);
    }

	void HistoryControlPage::quoteText(QString text)
	{
        auto selected_text = messages_area_->getSelectedText();

        if (selected_text.isEmpty())
            selected_text = text;

        QStringList lines = selected_text.split('\n');
        selected_text.clear();
        for (auto line : lines)
        {
            if (!line.isEmpty())
                selected_text += ">";
            selected_text += line;
            selected_text += "\n";
        }

		emit quote(selected_text);
	}

	void HistoryControlPage::updateNewPlate(bool close)
	{
		Logic::GetMessagesModel()->updateNew(aimId_, new_plate_position_, close);

		Data::DlgState state = Logic::GetRecentsModel()->getDlgState(aimId_, !close);
		if (close)
		{
			new_plate_position_ = state.LastMsgId_;
		}
		else
		{
			new_plate_position_ = state.UnreadCount_ == 0 ? -1 : state.YoursLastRead_;
		}
	}

	qint64 HistoryControlPage::getNewPlateId() const
	{
		return new_plate_position_;
	}

	void HistoryControlPage::sourceReady(QString aimId)
	{
        if (aimId != aimId_)
		{
			return;
		}

        assert(isStateIdle());
        assert(items_data_.empty());

        switchToFetchingState(__FUNCLINEA__);

        auto widgets = Logic::GetMessagesModel()->tail(aimId_, messages_area_, new_plate_position_);

        QMapIterator<Logic::MessageKey, QWidget*> iter(widgets);
		iter.toBack();
		while (iter.hasPrevious())
		{
			iter.previous();
            items_data_.emplace_back(iter.key(), iter.value(), Logic::MessagesModel::REQUESTED);
		}

        if (!items_data_.empty())
        {
            switchToInsertingState(__FUNCLINEA__);

            postInsertNextMessageSignal(__FUNCLINEA__);
        }

		if (widgets.count() < (signed)Logic::GetMessagesModel()->preloadCount())
        {
			requestMoreMessagesAsync(__FUNCLINEA__);
        }

		Logic::GetMessagesModel()->updateNew(aimId_, new_plate_position_);
	}

    void HistoryControlPage::insertNextMessageSlot()
	{
        __INFO(
            "smooth_scroll",
            "entering signal handler\n"
            "    type=<insertNextMessageSlot>\n"
            "    state=<" << state_ << ">\n"
            "    items size=<" << items_data_.size() << ">"
        );

        assert(isStateInserting());

		if (items_data_.empty())
        {
            switchToIdleState(__FUNCLINEA__);

            if (is_messages_request_postponed_)
            {
                __INFO(
                    "smooth_scroll",
                    "resuming postponed messages request\n"
                    "    requested at=<" << dbg_where_postponed_ << ">"
                );

                is_messages_request_postponed_ = false;
                dbg_where_postponed_ = nullptr;

                requestMoreMessagesAsync(__FUNCLINEA__);
            }

            return;
        }

        auto data = items_data_.front();
        items_data_.pop_front();

		__INFO(
			"history_control",
			"inserting widget\n"
			"	key=<" << data.Key_.Id_ << ";" << data.Key_.InternalId_ << ">");

        auto update_new_plate = [&data, this]()
        {
            if (messages_area_->isScrollAtBottom())
            {
                new_messages_plate_->setUnreadCount(0);
                new_messages_plate_->hide();
            }
            else
            {
                if (data.Mode_ == Logic::MessagesModel::BASE)
                {
                    if (data.Key_.isOutgoing())
                         messages_area_->scrollToBottom();

                    new_messages_plate_->addUnread();
                    new_messages_plate_->show();
                }
            }
        };

		QWidget* existing = getWidgetByKey(data.Key_);
		if (existing)
		{
			if (isPersistentWidget(existing))
			{
				__INFO(
					"history_control",
					"widget insertion discarded (persistent widget)\n"
					"	key=<" << data.Key_.Id_ << ";" << data.Key_.InternalId_ << ">");
				data.Widget_->deleteLater();

                postInsertNextMessageSignal(__FUNCLINEA__);

				return;
			}

			if (existing->property("New").toBool() || data.Widget_->property("New").toBool())
			{
				removeExistingWidgetByKey(data.Key_);
                recalculateWidgetsPositions();
			}
			else
			{
				auto messageItem = qobject_cast<Ui::MessageItem*>(existing);
				auto newMessageItem = qobject_cast<Ui::MessageItem*>(data.Widget_);

				if (messageItem && newMessageItem)
				{
					bool updated = messageItem->updateData(newMessageItem->getData());
                    (void)updated;
					data.Widget_->deleteLater();

                    if (!data.Key_.isOutgoing())
                    {
                        typingChattersAimIds_.remove(newMessageItem->getMchatSenderAimId());
                        if (typingChattersAimIds_.empty())
                            hideTypingWidgets();
                        else
                            updateTypingWidgets();
                    }

                    postInsertNextMessageSignal(__FUNCLINEA__);
                    update_new_plate();

					return;
				}

				if (data.Key_.isDate() || data.Key_.Type_ == core::message_type::chat_event)
				{
                    data.Widget_->deleteLater();

                    postInsertNextMessageSignal(__FUNCLINEA__);

					return;
				}

                if (data.Key_.Type_ == core::message_type::voip_event)
                {
                    removeExistingWidgetByKey(data.Key_);
                    recalculateWidgetsPositions();
                }
			}
		}

		__TRACE(
			"history_control",
			"inserting widget position info\n"
			"	key=<" << data.Key_.Id_ << ";" << data.Key_.InternalId_ << ">");

		auto newEntry = std::make_shared<PositionInfo>(data.Key_, data.Widget_);
        if (!position_list_.empty())
        {
            if (data.Key_ < position_list_.front()->toMessageKey())
            {
                position_list_.emplace_front(std::move(newEntry));
            }
            else if (position_list_.back()->toMessageKey() < data.Key_)
            {
                position_list_.emplace_back(std::move(newEntry));
            }
            else
            {
                if (data.Key_.Id_ == -1 && !data.Key_.InternalId_.isEmpty())
                {
                    std::list<PositionInfoSptr>::reverse_iterator iter = position_list_.rbegin();
                    while (iter != position_list_.rend())
                    {
                        if (data.Key_.PendingId_ < (*iter)->toMessageKey().PendingId_)
                        {
                            ++iter;
                            continue;
                        }

                        position_list_.emplace(iter.base(), std::move(newEntry));
                        break;
                    }
                }
                else
                {
                    std::list<PositionInfoSptr>::iterator iter = position_list_.begin();
                    while (iter != position_list_.end())
                    {
                        if ((*iter)->toMessageKey() < data.Key_)
                        {
                            ++iter;
                            continue;
                        }

                        position_list_.emplace(iter, std::move(newEntry));
                        break;
                    }
                }
            }
        }
        else
        {
            position_list_.emplace_back(std::move(newEntry));
        }

        recalculateWidgetsPositions();

		// prepare widget for insertion

		auto messageItem = qobject_cast<Ui::MessageItem*>(data.Widget_);
		if (messageItem)
		{
			connect(messageItem, SIGNAL(copy(QString)), this, SLOT(copy(QString)), Qt::QueuedConnection);
            connect(messageItem, SIGNAL(quote(QString)), this, SLOT(quoteText(QString)), Qt::QueuedConnection);
            if (!data.Key_.isOutgoing())
            {
                typingChattersAimIds_.remove(messageItem->getMchatSenderAimId());
                if (typingChattersAimIds_.empty())
                    hideTypingWidgets();
                else
                    updateTypingWidgets();
            }
		}
        else if (data.Widget_->layout()) // fix for fillNew
        {
            auto layout = data.Widget_->layout();

            auto index = 0;
            while (auto child = layout->itemAt(index++))
            {
                if (auto messageItem = qobject_cast<Ui::MessageItem*>(child->widget()))
                {
                    connect(messageItem, SIGNAL(copy(QString)), this, SLOT(copy(QString)), Qt::QueuedConnection);
                    connect(messageItem, SIGNAL(quote(QString)), this, SLOT(quoteText(QString)), Qt::QueuedConnection);
                }
            }
        }

		// insert and display the widget

		const auto widgetPosInfo = getPositionInfoByKey(data.Key_);
        assert(widgetPosInfo != position_list_.end());

        const auto widgetPos = (*widgetPosInfo)->getPosition();
        assert(widgetPos >= 0);

        messages_area_->insertWidget(widgetPos, data.Widget_);

        #pragma message(__TODOA__ "selection, muthafookah!")
		//if (event_filter_->selectMode() && messageItem)
		//	messageItem->selectByPos(mapToGlobal(rect().bottomRight()));

		__TRACE(
			"history_control",
			"new widget inserted\n"
			"	key=<" << data.Key_.Id_ << ";" << data.Key_.InternalId_ << ">\n" <<
			"	layout_pos=<" << widgetPos << ">");

        update_new_plate();
        postInsertNextMessageSignal(__FUNCLINEA__);
	}

	void HistoryControlPage::removeWidget(Logic::MessageKey key)
	{
		if (isScrolling())
		{
			emit needRemove(key);
			return;
		}

		__TRACE(
			"history_control",
			"requested to remove the widget\n"
			"	key=<" << key.Id_ << ";" << key.InternalId_ << ">");


        remove_requests_.erase(key);
		const auto result = removeExistingWidgetByKey(key);
        if (result <= WidgetRemovalResult::Min || result >= WidgetRemovalResult::Max)
		    assert(false);

        unloadWidgets();

        recalculateWidgetsPositions();
	}

    bool HistoryControlPage::touchScrollInProgress() const
    {
        return messages_area_->touchScrollInProgress();
    }

	bool HistoryControlPage::anySelected()
	{
		/*int i = 0;
		while (QLayoutItem* item = messages_area_->getMessagesLayout()->itemAt(i))
		{
			if (item->widget())
			{
				Ui::MessageItem* messageItem = qobject_cast<Ui::MessageItem*>(item->widget());
				if (messageItem && messageItem->selected())
					return true;
			}
			++i;
		}*/
		return false;
	}

    void HistoryControlPage::needCleanup()
    {
        unloadWidgets();
    }

	void HistoryControlPage::unloadWidgets()
	{
		Logic::MessageKey lastKey;

		auto count = (int)position_list_.size();
		count -= (Logic::GetMessagesModel()->preloadCount() + remove_requests_.size() + items_data_.size());

		for (const auto &pos : position_list_)
		{
			if (count-- <= 0)
			{
				break;
			}

			if (lastKey.isEmpty() && remove_requests_.find(pos->toMessageKey()) == remove_requests_.end())
			{
				lastKey = pos->toMessageKey();
				emit needRemove(lastKey);
				remove_requests_.insert(lastKey);
				break;
			}
		}

		if (!lastKey.isEmpty())
		{
			Logic::GetMessagesModel()->setLastKey(lastKey, aimId_);
		}
	}

    void HistoryControlPage::loadChatInfo(bool is_full_list_loaded_)
    {
        chat_info_sequence_ = Logic::ChatMembersModel::load_all_members(aimId_, is_full_list_loaded_ ? Logic::MaxMembersLimit : Logic::InitMembersLimit, this);
    }

	void HistoryControlPage::initStatus()
	{
		Logic::ContactItem* contact = Logic::GetContactListModel()->getContactItem(aimId_);
		if (!contact)
			return;

		if (contact->is_chat())
		{
            loadChatInfo(false);
        }
		else
		{
			QString state;
			QDateTime lastSeen = contact->Get()->LastSeen_;
			if (lastSeen.isValid())
			{
				state = QT_TRANSLATE_NOOP("chat_page","Seen ");

				const auto current = QDateTime::currentDateTime();

				const auto days = lastSeen.daysTo(current);

				if (days == 0)
				{
					state += QT_TRANSLATE_NOOP("chat_page", "today");

				}
				else if (days == 1)
				{
					state += QT_TRANSLATE_NOOP("chat_page", "yesterday");
				}
				else
				{
					state += Utils::GetTranslator()->formatDate(lastSeen.date(), lastSeen.date().year() == current.date().year());
				}

				if (lastSeen.date().year() == current.date().year())
				{
					state += QT_TRANSLATE_NOOP("chat_page", " at ");
					state += lastSeen.time().toString(Qt::SystemLocaleShortDate);
				}
			}
			else
			{
				state = contact->is_phone() ? contact->Get()->AimId_ : (contact->Get()->StatusMsg_.isEmpty() ? contact->Get()->State_ : contact->Get()->StatusMsg_);
			}
			contact_status_->setText(state);
		}
	}

    void HistoryControlPage::updateName()
    {
        Logic::ContactItem* contact = Logic::GetContactListModel()->getContactItem(aimId_);
        event_filter_->ResetContactName(contact->Get()->GetDisplayName());
    }

    void HistoryControlPage::updateMenu(QString aimId)
    {
        if (aimId != aimId_)
            return;

        auto active_item = Logic::GetContactListModel()->getContactItem(aimId_);
        assert(active_item);

        menu_->clear();
		if (active_item->is_not_auth())
			menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_addpers_100.png")), QT_TRANSLATE_NOOP("context_menu", "Add contact"), makeData("add"));
		if (Logic::GetContactListModel()->isMuted(aimId_))
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_unmute_100.png")), QT_TRANSLATE_NOOP("context_menu","Turn on notifications"), makeData("unmute"));
        else
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_mute_100.png")), QT_TRANSLATE_NOOP("context_menu", "Turn off notifications"), makeData("mute"));
        //Menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_closechat_100.png")), QT_TRANSLATE_NOOP("context_menu","Delete history"), makeData("delete_history"));
        if (!active_item->is_chat())
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_profile_100.png")), QT_TRANSLATE_NOOP("context_menu", "Profile"), makeData("Profile"));

		if ((!active_item->is_chat() || is_chat_member_) && (!active_item->is_not_auth()))
			menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_addpers_100.png")), QT_TRANSLATE_NOOP("context_menu", "Add to chat"), makeData("add_members"));

		menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_ignore_100.png")), QT_TRANSLATE_NOOP("context_menu", "Ignore"), makeData("ignore"));

        if (active_item->is_chat())
        {
            if (chat_members_model_ != NULL && !is_public_chat_)
                menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_rename_100.png")), QT_TRANSLATE_NOOP("context_menu", "Rename"), makeData("rename"));
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_delete_100.png")), QT_TRANSLATE_NOOP("context_menu", "Quit and delete"), makeData("remove"));
        }
        else
        {
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_spam_100.png")), QT_TRANSLATE_NOOP("context_menu", "Report spam"), makeData("spam"));
            menu_->addActionWithIcon(QIcon(Utils::parse_image_name(":/resources/dialog_delete_100.png")), QT_TRANSLATE_NOOP("context_menu", "Delete"), makeData("remove"));
        }
    }

    void HistoryControlPage::chatInfoFailed(qint64 seq, core::group_chat_info_errors _error_code)
    {
        if (Logic::ChatMembersModel::receive_members(chat_info_sequence_, seq, this))
        {
            SetContactStatusClickable(false);
            if (_error_code == core::group_chat_info_errors::not_in_chat)
            {
                contact_status_->setText(QT_TRANSLATE_NOOP("groupchat_pages","You are not a member of this groupchat"));
                add_member_button_->hide();
                edit_members_button_->hide();
            }
        }
    }

	void HistoryControlPage::chatInfo(qint64 seq, std::shared_ptr<Data::ChatInfo> info)
	{
		if (Logic::ChatMembersModel::receive_members(chat_info_sequence_, seq, this))
        {
            event_filter_->ResetContactName(info->Name_);
            SetContactStatusClickable(true);

            QString state = QString("%1").arg(info->MembersCount_) + QString(" ") + Utils::GetTranslator()->getNumberString(info->MembersCount_, QT_TRANSLATE_NOOP3("chat_page", "member", "1"), QT_TRANSLATE_NOOP3("chat_page", "members", "2"),
				QT_TRANSLATE_NOOP3("chat_page", "members", "5"), QT_TRANSLATE_NOOP3("chat_page", "members", "21"));
			contact_status_->setText(state);
            is_public_chat_ = info->Public_;
            if (chat_members_model_ == NULL)
            {
                chat_members_model_ = new Logic::ChatMembersModel(info, this);
            }
            else
            {
                chat_members_model_->update_info(info, false);
                emit updateMembers();
            }
            updateMenu(aimId_);
		}
	}

	void HistoryControlPage::contactChanged(QString aimid)
	{
		if (aimid == aimId_)
        {
			initStatus();
            updateName();
        }
	}

	void HistoryControlPage::open()
	{
		initStatus();
	}

    bool HistoryControlPage::requestMoreMessagesAsync(const char *dbgWhere)
    {
        if (isStateFetching())
        {
            __INFO(
                "smooth_scroll",
                "requesting more messages\n"
                "    status=<cancelled>\n"
                "    reason=<already fetching>\n"
                "    from=<" << dbgWhere << ">"
            );

            return false;
        }

        if (isStateInserting())
        {
            __INFO(
                "smooth_scroll",
                "requesting more messages\n"
                "    status=<postponed>\n"
                "    reason=<inserting>\n"
                "    from=<" << dbgWhere << ">"
            );

            postponeMessagesRequest(dbgWhere);

            return true;
        }

        __INFO(
            "smooth_scroll",
            "requesting more messages\n"
            "    status=<requested>\n"
            "    from=<" << dbgWhere << ">"
        );

        switchToFetchingState(__FUNCLINEA__);

        emit requestMoreMessagesSignal();

        return true;
    }

    QString HistoryControlPage::aimId() const
    {
        return aimId_;
    }

    void HistoryControlPage::cancelSelection()
    {
        assert(messages_area_);
        messages_area_->cancelSelection();
    }

	void HistoryControlPage::messageKeyUpdated(QString aimId, Logic::MessageKey key)
	{
		assert(key.hasId());

		if (aimId != aimId_)
		{
			return;
		}

		__TRACE(
			"history_control",
			"incoming message server key\n"
			"	internal_id=<" << key.InternalId_ << ">\n"
			"	server_id=<" << key.Id_ << ">");

        auto existingPosIter = getPositionInfoByKey(key);
        if (existingPosIter == position_list_.end())
        {
            __WARN(
                "history_control",
                "updated key belongs to no message in history control\n"
                "	internal_id=<" << key.InternalId_ << ">\n"
                "	server_id=<" << key.Id_ << ">");
            return;
        }

        auto existingPos = *existingPosIter;
        existingPos->setMessageKey(key);
	}

	void HistoryControlPage::updated(QList<Logic::MessageKey> list, QString aimId, unsigned mode)
	{
		if (aimId != aimId_)
        {
			return;
        }

        const auto isHole = (mode == Logic::MessagesModel::HOLE);
        if (isHole)
        {
			event_filter_->resetNewPlate();
        }

		for (auto key : list)
		{
            QWidget* msg = Logic::GetMessagesModel()->getById(aimId_, key, messages_area_, new_plate_position_);
			if (msg)
            {
                items_data_.emplace_back(key, msg, mode);

                if (key.Type_ == core::message_type::chat_event)
                {
                    updateChatInfo();
                }
            }
		}

        if (!items_data_.empty() && !isStateInserting())
        {
            if (isStateIdle())
            {
                switchToFetchingState(__FUNCLINEA__);
            }

            switchToInsertingState(__FUNCLINEA__);

            postInsertNextMessageSignal(__FUNCLINEA__);
        }

        if (!messages_area_->isViewportFull())
        {
            requestMoreMessagesAsync(__FUNCLINEA__);
        }
	}

	void HistoryControlPage::deleted(QList<Logic::MessageKey> list, QString aimId)
	{
		if (aimId != aimId_)
        {
			return;
        }

		for (auto keyToRemove : list)
		{
			removeExistingWidgetByKey(keyToRemove);

            for (auto itemIter = items_data_.cbegin(); itemIter != items_data_.cend();)
            {
                if (itemIter->Key_ != keyToRemove)
                {
                    ++itemIter;
                    continue;
                }

                itemIter = items_data_.erase(itemIter);
            }
		}

        recalculateWidgetsPositions();
	}

	void HistoryControlPage::requestMoreMessagesSlot()
	{
        assert(isStateFetching());

        if (!isVisible())
        {
			return;
        }

		auto widgets = Logic::GetMessagesModel()->more(aimId_, messages_area_, new_plate_position_);

        QMapIterator<Logic::MessageKey, QWidget*> iter(widgets);
		iter.toBack();
		while (iter.hasPrevious())
		{
			iter.previous();
            items_data_.emplace_back(iter.key(), iter.value(), Logic::MessagesModel::REQUESTED);
		}

        switchToInsertingState(__FUNCLINEA__);

        postInsertNextMessageSignal(__FUNCLINEA__);

        if (!messages_area_->isViewportFull())
        {
            requestMoreMessagesAsync(__FUNCLINEA__);
        }
	}

	void HistoryControlPage::downPressed()
	{
		new_messages_plate_->hide();
        new_messages_plate_->setUnreadCount(0);

		messages_area_->scrollToBottom();
	}

    void HistoryControlPage::scrollMovedToBottom()
    {
        new_messages_plate_->hide();
        new_messages_plate_->setUnreadCount(0);
    }


	void HistoryControlPage::autoScroll(bool enabled)
	{
		if (!enabled)
		{
			return;
		}

	//	unloadWidgets();
		new_messages_plate_->setUnreadCount(0);
		new_messages_plate_->hide();
	}

	void HistoryControlPage::callAudioButtonClicked() {
		//#ifdef _WIN32
		Ui::GetDispatcher()->getVoipController().setStartA(aimId_.toUtf8(), false);
        if (MainPage* mainPage = MainPage::instance()) {
            mainPage->raiseVideoWindow();
        }

		//#endif
	}
	void HistoryControlPage::callVideoButtonClicked() {
		//#ifdef _WIN32
		Ui::GetDispatcher()->getVoipController().setStartV(aimId_.toUtf8(), false);
        if (MainPage* mainPage = MainPage::instance()) {
            mainPage->raiseVideoWindow();
        }
		//#endif
	}

	void HistoryControlPage::moreButtonClicked()
	{
        QPoint p = more_button_->mapToGlobal(more_button_->rect().bottomRight());
		menu_->popup(p);
	}

	void HistoryControlPage::focusOutEvent(QFocusEvent* _event)
	{
		QWidget::focusOutEvent(_event);
	}

    void HistoryControlPage::wheelEvent(QWheelEvent* _event)
    {
        if (!hasFocus())
        {
            return;
        }

        return QWidget::wheelEvent(_event);
    }

	void HistoryControlPage::add_member()
	{
        if (!Logic::GetContactListModel()->getContactItem(aimId_)->is_chat() && !chat_members_model_)
            return;

        GetDispatcher()->post_stats_to_core(core::stats::stats_event_names::groupchat_from_dialog);

        assert(!!chat_members_model_);

        Logic::SetChatMembersModel(chat_members_model_);
		SemitransparentWindow window(Ui::get_gui_settings(), this);

        if (!chat_members_model_->is_full_list_loaded_)
        {
            chat_members_model_->load_all_members();
        }

		SelectContactsWidget select_members_dialog_(NULL, Logic::MembersWidgetRegim::SELECT_MEMBERS, QT_TRANSLATE_NOOP("groupchat_pages", "Add to groupchat"), QT_TRANSLATE_NOOP("groupchat_pages", "Done"), Ui::get_gui_settings(), this);
        connect(this, SIGNAL(updateMembers()), &select_members_dialog_, SLOT(UpdateMembers()), Qt::QueuedConnection);

		if (select_members_dialog_.show() == QDialog::Accepted)
		{
			onFinishAddMembers(true);
		}
		else
		{
			clearAddMembers();
		}
        Logic::SetChatMembersModel(NULL);
	}

    void HistoryControlPage::rename_chat()
    {
        QString old_chat_name = event_filter_->getContactName();
        QString result_chat_name;
        auto result = SelectContactsWidget::ChatNameEditor(old_chat_name, &result_chat_name, this, QT_TRANSLATE_NOOP("groupchat_pages","Save"));
        if (result)
        {
            Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);
            result_chat_name = result_chat_name.isEmpty() ? old_chat_name : result_chat_name;

            collection.set_value_as_string("aimid", aimId_.toUtf8().data(), aimId_.toUtf8().size());
            collection.set_value_as_string("m_chat_name", result_chat_name.toUtf8().data(), result_chat_name.toUtf8().size());
            Ui::GetDispatcher()->post_message_to_core("modify_chat", collection.get());
        }
    }

    void HistoryControlPage::setState(const State state, const char *dbgWhere)
    {
        assert(state > State::Min);
        assert(state < State::Max);
        assert(state_ > State::Min);
        assert(state_ < State::Max);

        __INFO(
            "smooth_scroll",
            "switching state\n"
            "    from=<" << state_ << ">\n"
            "    to=<" << state << ">\n"
            "    where=<" << dbgWhere <<">\n"
            "    items num=<" << items_data_.size() << ">"
        );

        state_ = state;
    }

    bool HistoryControlPage::isState(const State state) const
    {
        assert(state_ > State::Min);
        assert(state_ < State::Max);
        assert(state > State::Min);
        assert(state < State::Max);

        return (state_ == state);
    }

    bool HistoryControlPage::isStateFetching() const
    {
        return isState(State::Fetching);
    }

    bool HistoryControlPage::isStateIdle() const
    {
        return isState(State::Idle);
    }

    bool HistoryControlPage::isStateInserting() const
    {
        return isState(State::Inserting);
    }

    void HistoryControlPage::postInsertNextMessageSignal(const char *dbgWhere)
    {
        __INFO(
            "smooth_scroll",
            "posting signal\n"
            "    type=<insertNextMessageSignal>\n"
            "    from=<" << dbgWhere << ">"
        );

        emit insertNextMessageSignal();
    }

    void HistoryControlPage::postponeMessagesRequest(const char *dbgWhere)
    {
        assert(isStateInserting());

        dbg_where_postponed_ = dbgWhere;
        is_messages_request_postponed_ = true;
    }

    void HistoryControlPage::switchToIdleState(const char *dbgWhere)
    {
        assert(isStateInserting());

        setState(State::Idle, dbgWhere);
    }

    void HistoryControlPage::switchToInsertingState(const char *dbgWhere)
    {
        assert(isStateFetching());

        setState(State::Inserting, dbgWhere);
    }

    void HistoryControlPage::switchToFetchingState(const char *dbgWhere)
    {
        assert(isStateIdle() || isStateInserting());

        setState(State::Fetching, dbgWhere);
    }

	void HistoryControlPage::onFinishAddMembers(bool _isAccept)
	{
		if (_isAccept)
		{
			auto selectedContacts = Logic::GetContactListModel()->GetCheckedContacts();
			Logic::GetContactListModel()->ClearChecked();

			Ui::gui_coll_helper collection(Ui::GetDispatcher()->create_collection(), true);

			QStringList chat_members;
			for (const auto& contact : selectedContacts)
			{
				chat_members.push_back(contact.get_aimid());
			}
			collection.set_value_as_string("aimid", aimId_.toUtf8().data(), aimId_.toUtf8().size());
			collection.set_value_as_string("m_chat_members_to_add", chat_members.join(";").toStdString());
			Ui::GetDispatcher()->post_message_to_core("add_members", collection.get());
		}
	}

	void HistoryControlPage::clearAddMembers()
	{
		Logic::GetContactListModel()->ClearChecked();
		// pages_->setCurrentWidget(contactDialog_);
		// contactlListWidget_->update();
	}

	void HistoryControlPage::edit_members()
	{
        if (!chat_members_model_)
            return;

        contact_status_widget_->setStyleSheet("background-color: rgba(202,230,179,40%);");

        Logic::SetChatMembersModel(chat_members_model_);
		SelectContactsWidget select_members_dialog_(chat_members_model_, Logic::MembersWidgetRegim::DELETE_MEMBERS, contact_name_->toPlainText(), QT_TRANSLATE_NOOP("groupchat_pages", "Done"), Ui::get_gui_settings(), this);
        connect(this, SIGNAL(updateMembers()), &select_members_dialog_, SLOT(UpdateMembers()), Qt::QueuedConnection);

        auto x = contact_status_->mapToGlobal(contact_status_->pos()).x() - ::ContactList::dip(19).px();
        auto y = contact_status_->mapToGlobal(edit_members_button_->pos()).y() + ::ContactList::dip(15).px();
		select_members_dialog_.show(x, y);
        Logic::SetChatMembersModel(NULL);
        contact_status_widget_->setStyleSheet("background-color: transparent;");
	}

    void HistoryControlPage::updateChatInfo()
    {
        if (chat_members_model_ == NULL)
        {
            loadChatInfo(false);
        }
        else
        {
            loadChatInfo(chat_members_model_->is_full_list_loaded_);
        }
    }

    void HistoryControlPage::onReachedFetchingDistance()
    {
        __INFO(
            "smooth_scroll",
            "initiating messages preloading..."
        );

        requestMoreMessagesAsync(__FUNCLINEA__);
    }

	void HistoryControlPage::popup_menu(QAction* _action)
	{
		auto params = _action->data().toMap();
		const QString command = params["command"].toString();

		if (command == "add")
		{
			auth_add_contact(aimId_);
		}

		if (command == "mute")
		{
			Logic::GetRecentsModel()->muteChat(aimId_, true);
		}
		else if (command == "unmute")
		{
			Logic::GetRecentsModel()->muteChat(aimId_, false);
		}
		else if (command == "ignore")
		{
			Logic::GetContactListModel()->ignore_contact(aimId_, true);
			Logic::GetContactListModel()->remove_contact_from_contact_list(aimId_);
		}
		else if (command == "delete_history")
		{

		}
		else if (command == "Profile")
		{
			emit Utils::InterConnector::instance().profileSettingsShow(aimId_);
		}
		else if (command == "spam")
		{
			Logic::GetContactListModel()->block_spam_contact(aimId_);
			Logic::GetContactListModel()->remove_contact_from_contact_list(aimId_);
		}
		else if (command == "remove")
		{
			Logic::GetContactListModel()->remove_contact_from_contact_list(aimId_);
        }
        else if (command == "rename")
        {
            rename_chat();
        }
        else if (command == "add_members")
        {
            if (Logic::GetContactListModel()->getContactItem(aimId_)->is_chat())
            {
                add_member();
            }
            else
            {
                QStringList members;
                members.append(aimId_);
                MainPage::instance()->createGroupChat(members);
            }
        }
	}

    void HistoryControlPage::SetContactStatusClickable(bool _is_enabled)
    {
        if (_is_enabled == is_contact_status_clickable_)
            return;

        if (_is_enabled)
        {
            is_chat_member_ = true;
            contact_status_widget_->setCursor(Qt::PointingHandCursor);
            edit_members_button_->setVisible(true);
            connect(contact_status_, SIGNAL(clicked()), this, SLOT(edit_members()), Qt::QueuedConnection);
            connect(edit_members_button_, SIGNAL(clicked()), this, SLOT(edit_members()), Qt::QueuedConnection);
        }
        else
        {
            is_chat_member_ = false;
            contact_status_widget_->setCursor(Qt::ArrowCursor);
            edit_members_button_->setVisible(false);
            disconnect(contact_status_, SIGNAL(clicked()), this, SLOT(edit_members()));
            disconnect(edit_members_button_, SIGNAL(clicked()), this, SLOT(edit_members()));
        }
        updateMenu(aimId_);
        is_contact_status_clickable_ = _is_enabled;
    }

    void HistoryControlPage::showEvent(QShowEvent* _event)
    {
        appendAuthControlIfNeed();

        QWidget::showEvent(_event);
    }
}

namespace
{
	bool isPersistentWidget(QWidget *w)
	{
		const auto messageItem = qobject_cast<Ui::MessageItem*>(w);
		if (!messageItem)
		{
			return false;
		}

		return messageItem->isPersistent();
	}

	QMap<QString, QVariant> makeData(const QString& command, const QString& aimid)
	{
		QMap<QString, QVariant> result;
		result["command"] = command;
		result["contact"] = aimid;
		return result;
	}
}