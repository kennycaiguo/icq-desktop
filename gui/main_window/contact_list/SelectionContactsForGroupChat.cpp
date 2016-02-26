#include "stdafx.h"
#include "SelectionContactsForGroupChat.h"
#include "../../utils/utils.h"
#include "ContactListModel.h"
#include "SearchModel.h"
#include "ContactListItemDelegate.h"
#include "ContactListItemRenderer.h"
#include "SearchWidget.h"
#include "ContactList.h"
#include "../../controls/GeneralDialog.h"
#include "../../controls/TextEditEx.h"
#include "../../gui_settings.h"
#include "Common.h"
#include "ChatMembersModel.h"
#include "../../controls/SemitransparentWindow.h"
#include "../../core_dispatcher.h"
#include "../../utils/gui_coll_helper.h"
#include "../../controls/TextEmojiWidget.h"
#include "../../controls/Alert.h"
#include "../settings/ProfileSettingsWidget.h"
#include "../../utils/InterConnector.h"

namespace Ui
{
    void SelectContactsWidget::deleteMemberDialog(Logic::ChatMembersModel* _model, const QString& current, int _regim)
    {
        QString label_style = "font-family: " + Utils::appFontFamily(Utils::FontsFamily::SEGOE_UI) + "; "
            + "font-size: 15dip; "
            + "color: rgba(105, 105, 105, 100%); "
            + "padding: 0 0 0 0dip; "
            + "padding-left: 24dip; "
            + "padding-right: 24dip; "
            + "margin: 0dip; ";

        auto get_button_style = [](QString _color, QString _background_color)
        {
            return "font-family: " + Utils::appFontFamily(Utils::FontsFamily::SEGOE_UI) + "; "
                +"font-size: 18dip; "
                +"padding: 0; "
                +"padding-left: 40dip; "
                +"padding-right: 40dip; "
                +"max-height: 40dip; "
                +"min-height: 40dip; "
                +"border-style: solid; border-color: #579e1c; border-width: 0dip;"
                +"color:" + _color + "; "
                +"background-color: " + _background_color + ";";
        };

        QString cancel_button_style =
            "QPushButton { " + get_button_style("#282828", "#dcdcdc") + "}"
            + "QPushButton:hover { " + get_button_style("#282828", "#e5e5e5") + "}"
            + "QPushButton:pressed { " + get_button_style("#282828", "#d5d5d5") + "}";

        QString remove_button_style =  "QPushButton { " + get_button_style("#579e1c", "rgba(202,230,179,40%)") + "}"
            + "QPushButton:hover { " + get_button_style("#ffffff", "#579e1c") + "}"
            + "QPushButton:pressed { " + get_button_style("#ffffff", "#579e1c") + "}";

        auto main_widget = new QWidget(this);
        auto layout = new QVBoxLayout(main_widget);
        layout->setAlignment(Qt::AlignTop);
        main_widget->setLayout(layout);

        QSpacerItem* upper_spacer = new QSpacerItem(0, 24, QSizePolicy::Minimum);
        layout->addSpacerItem(upper_spacer);

        auto label = new QLabel(this);

        if (_regim == Logic::MembersWidgetRegim::DELETE_MEMBERS)
            label->setText(QT_TRANSLATE_NOOP("popup_window", "Are you sure you want to remove user from this chat?"));
        else if (_regim == Logic::MembersWidgetRegim::IGNORE_LIST)
            label->setText(QT_TRANSLATE_NOOP("popup_window", "Are you sure you want to remove user from ignore list?"));

        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        label->setStyleSheet(Utils::ScaleStyle(label_style, Utils::get_scale_coefficient()));
        label->setWordWrap(true);
        layout->addWidget(label);

        auto bottom_layout = new QHBoxLayout();
        bottom_layout->setAlignment(Qt::AlignTop);
        QSpacerItem* horizontal_spacer_buttom1 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        QSpacerItem* horizontal_spacer_buttom2 = new QSpacerItem(::ContactList::dip(12).px(), 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
        QSpacerItem* horizontal_spacer_buttom3 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        auto cancel_button = new QPushButton(this);
        cancel_button->setCursor(QCursor(Qt::PointingHandCursor));
        cancel_button->setText(QT_TRANSLATE_NOOP("popup_window", "Cancel"));
        cancel_button->setStyleSheet(Utils::ScaleStyle(cancel_button_style, Utils::get_scale_coefficient()));

        auto remove_button = new QPushButton(this);
        remove_button->setCursor(QCursor(Qt::PointingHandCursor));
        remove_button->setText(QT_TRANSLATE_NOOP("popup_window", "Remove"));
        remove_button->setStyleSheet(Utils::ScaleStyle(remove_button_style, Utils::get_scale_coefficient()));

        bottom_layout->addItem(horizontal_spacer_buttom1);
        bottom_layout->addWidget(cancel_button);
        bottom_layout->addItem(horizontal_spacer_buttom2);
        bottom_layout->addWidget(remove_button);
        bottom_layout->addItem(horizontal_spacer_buttom3);

        QSpacerItem* middle_spacer = new QSpacerItem(0, 24, QSizePolicy::Minimum);
        layout->addSpacerItem(middle_spacer);
        layout->addLayout(bottom_layout);

        QSpacerItem* buttom_spacer = new QSpacerItem(0, 24, QSizePolicy::Minimum);
        layout->addSpacerItem(buttom_spacer);
        layout->setSpacing(0);
        layout->setMargin(0);

        layout->setContentsMargins(0, 0, 0, 0);

        auto member = _model->getMemberItem(current);
        auto user_name = member->NickName_;
        auto general_dialog = new GeneralDialog(true, false, user_name.isEmpty() ? member->AimdId_ : user_name,
            QT_TRANSLATE_NOOP("groupchat_pages", "Remove"), main_widget, parentWidget(), 24);
        auto main_rect = Ui::get_gui_settings()->get_value(settings_main_window_rect_real, QRect());
        auto main_width = main_rect.width();
        auto main_height = main_rect.height();

        auto new_width = std::max(::ContactList::ItemLength(true, 0.3, ::ContactList::dip(0)).px(), ::ContactList::dip(200).px());
        auto new_height = std::max(::ContactList::ItemLength(false, 0.3, ::ContactList::dip(0)).px(), ::ContactList::dip(200).px());

        auto x = main_rect.x() + main_width / 2 - new_width /2;
        auto y = main_rect.y() + main_height / 2 - new_height / 2;

        QObject::connect(cancel_button, SIGNAL(clicked()), general_dialog, SLOT(reject()), Qt::QueuedConnection);
        QObject::connect(remove_button, SIGNAL(clicked()), general_dialog, SLOT(accept()), Qt::QueuedConnection);

        SemitransparentWindow window(Ui::get_gui_settings(), parentWidget());
        Utils::setWidgetPopup(general_dialog, true);
        
        auto result = general_dialog->showWithFixedSizes(::ContactList::dip(380).px(), -1, x, y);
        if (result)
        {
            if (_regim == Logic::MembersWidgetRegim::DELETE_MEMBERS)
            {
                auto aimId_ = _model->get_chat_aimid();
                ::Ui::gui_coll_helper collection(::Ui::GetDispatcher()->create_collection(), true);
                collection.set_value_as_string("aimid", aimId_.toUtf8().data(), aimId_.toUtf8().size());
                collection.set_value_as_string("m_chat_members_to_remove", current.toStdString());
                Ui::GetDispatcher()->post_message_to_core("remove_members", collection.get());
            }
            else if (_regim == Logic::MembersWidgetRegim::IGNORE_LIST)
            {
                Logic::GetContactListModel()->ignore_contact(current, false);
                // NOTE : when delete from ignore list, we don't need add contact to CL
            }
        }
        delete general_dialog;
    }

	void SelectContactsWidget::SelectionChanged(const QString& _current)
	{
        if (Logic::is_delete_members(regim_))
        {
            auto cursor_pos = QCursor::pos();
            auto dialog_rect = main_dialog_->geometry();
            auto pos = cursor_pos.x() - dialog_rect.x();
            auto leftX = ::ContactList::GetXOfRemoveImg(false, true);
            if (pos > leftX)
            {
                deleteMemberDialog(chatMembersModel_, _current, regim_);
            }
            else
            {
                emit ::Utils::InterConnector::instance().profileSettingsShow(_current);
                show();
            }
        }
        else
        {
            auto chat_members = Logic::GetChatMembersModel();
            if (!!chat_members && chat_members->is_contact_in_chat(_current))
            {
                deleteMemberDialog(chat_members, _current, regim_);
                return;
            }
            else
            {
                Logic::GetContactListModel()->ChangeChecked(_current);
                main_dialog_->setButtonActive(Logic::GetContactListModel()->GetCheckedContacts().size() > 0);
                contactList_->update();
            }
        }
	}

	SelectContactsWidget::SelectContactsWidget(Logic::ChatMembersModel* _chatMembersModel, int _regim, const QString& _label_text,
        const QString& _button_text, qt_gui_settings* _qt_setting, QWidget* _parent)
		: QDialog(_parent)
		, qt_setting_(_qt_setting)
		, regim_(_regim)
        , chatMembersModel_(_chatMembersModel)
        , is_short_view_(false)
    {
        if (_chatMembersModel)
        {
           chatMembersModel_->is_short_view_ = true;
           is_short_view_ = true;
        }

        auto main_widget = new QWidget(this);

		auto global_layout = new QVBoxLayout(main_widget);
        global_layout->setContentsMargins(0, 0, 0, 0);
        global_layout->setMargin(0);
        global_layout->setSpacing(0);
        main_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

		searchWidget_ = new SearchWidget(16, true);
        Testing::setAccessibleName(searchWidget_, "CreateGroupChat");
        global_layout->addWidget(searchWidget_);
        
		contactList_ = new ContactList(this, (Logic::MembersWidgetRegim)regim_, _chatMembersModel);
        contactList_->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        global_layout->addWidget(contactList_);
        
		QSpacerItem* contactsLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum);
        global_layout->addSpacerItem(contactsLayoutSpacer);

        auto is_show_search = !Logic::is_delete_members(regim_);// || chatMembersModel_->NeedSearch();
        auto is_show_button = !Logic::is_delete_members(regim_);

        horizontalLineWidget_view_all = new QWidget(this);
        horizontalLineWidget_view_all->setFixedHeight(Utils::scale_value(1));
        QString line_style = QString("background-color: #dadada; margin-left: 16dip; margin-right: 16dip;");
        horizontalLineWidget_view_all->setStyleSheet(Utils::ScaleStyle(line_style, Utils::get_scale_coefficient()));
        global_layout->addWidget(horizontalLineWidget_view_all);

        view_all_spacer1 = new QSpacerItem(1, Utils::scale_value(10));
        global_layout->addSpacerItem(view_all_spacer1);

        view_all = new TextEmojiLabel(this);
        if (Logic::is_delete_members(regim_))
            view_all->setText(QT_TRANSLATE_NOOP("groupchat_pages","View all ") + "("+ QString::number(_chatMembersModel->get_members_count()) +")");
        QString view_all_style = "font-family: " + Utils::appFontFamily(Utils::FontsFamily::SEGOE_UI) + "; "
                +"padding: 0 0 0 0dip; "
                +"font-size: 15dip; "
                +"color: #579e1c";
		view_all->setCursor(Qt::PointingHandCursor);
		view_all->setStyleSheet(Utils::ScaleStyle(view_all_style, Utils::get_scale_coefficient()));
        view_all->setMargin(0);
        view_all->setContentsMargins(0, 0, 0, 0);
        if (!platform::is_apple())
            view_all->setSizeToBaseline(Utils::scale_value(18));
        view_all->setSizeToOffset(Utils::scale_value(58));
        global_layout->addWidget(view_all);

        view_all_spacer2 = new QSpacerItem(1, Utils::scale_value(12));
        global_layout->addSpacerItem(view_all_spacer2);

        connect(view_all, SIGNAL(clicked()), this, SLOT(onViewAllMembers()), Qt::QueuedConnection);

        if (!is_show_search)
            searchWidget_->setVisible(false);

        if (!Logic::is_delete_members(regim_) || _chatMembersModel->get_members_count() <= ::Logic::InitMembersLimit)
        {
            horizontalLineWidget_view_all->setVisible(false);
            view_all->setVisible(false);
            view_all_spacer1->changeSize(1, 0);
            view_all_spacer2->changeSize(1, 0);
        }
        // TODO : use SetView here

		main_dialog_ = new GeneralDialog(is_show_search, is_show_button, _label_text, _button_text, main_widget, _parent, 16);

        Testing::setAccessibleName(main_dialog_, "SelectContactsWidget");

		connect(contactList_, SIGNAL(itemSelected(const QString&)), this, SLOT(SelectionChanged(const QString&)), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(searchBegin()), this, SLOT(searchBegin()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(searchEnd()), this, SLOT(searchEnd()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(enterPressed()), contactList_, SLOT(searchResult()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(upPressed()), contactList_, SLOT(searchUpPressed()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(downPressed()), contactList_, SLOT(searchDownPressed()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(search(QString)), Logic::GetCurrentSearchModel(regim_), SLOT(searchPatternChanged(QString)), Qt::QueuedConnection);
	//	connect(searchWidget_, SIGNAL(addNewGroupChat()), this, SLOT(createGroupChat()), Qt::QueuedConnection);
		connect(contactList_, SIGNAL(searchEnd()), searchWidget_, SLOT(searchCompleted()), Qt::QueuedConnection);
		connect(searchWidget_, SIGNAL(searchEnd()), this, SLOT(searchEnd()), Qt::QueuedConnection);

        if (Logic::is_delete_members(regim_))
            connect(contactList_, SIGNAL(addContactClicked()), this, SLOT(onViewAllMembers()), Qt::QueuedConnection);

		contactList_->changeTab(Ui::CurrentTab::SEARCH);
		Logic::GetCurrentSearchModel(regim_)->searchPatternChanged("");
		searchWidget_->SetShowButton(false);
	}

	SelectContactsWidget::~SelectContactsWidget()
	{
        delete main_dialog_;
	}

	void SelectContactsWidget::searchEnd()
	{
        Logic::GetCurrentSearchModel(regim_)->searchPatternChanged("");
		//contactList_->setSearchMode(false);
	}

	void SelectContactsWidget::searchBegin()
	{
		contactList_->setSearchMode(true);
	}

    bool SelectContactsWidget::show()
    {
        return show(default_invalid_coord, default_invalid_coord);
    }

    QRect* SelectContactsWidget::GetSizesAndPosition(int _x, int _y, bool _is_short_view)
    {
        int new_width = -1;

        if (Logic::is_delete_members(regim_))
            new_width = ::ContactList::ItemWidth(false, false, is_short_view_).px();
        else
            new_width = ::ContactList::ItemWidth(false, true, false).px();

        auto is_show_search = !Logic::is_delete_members(regim_) || !is_short_view_;
        double koeff_height = !is_show_search ? 0.4 : 0.8;

        auto new_height = ::ContactList::ItemLength(false, koeff_height, ::ContactList::dip(0)).px();
        auto add_height = (is_show_search ? searchWidget_->sizeHint().height() : 0);
        if (!Logic::is_delete_members(regim_))
            add_height += ::ContactList::dip(42).px();

        if (Logic::is_delete_members(regim_)
            && is_short_view_
            && chatMembersModel_->get_members_count() >= Logic::InitMembersLimit)
            add_height += ::ContactList::dip(52).px();

        auto itemHeight = ::ContactList::ContactItemHeight();
        int count = (new_height - add_height) / itemHeight;

        int cl_height = (count + 0.5) * itemHeight;
        if (Logic::is_delete_members(regim_))
        {
            int count = chatMembersModel_->rowCount();
            cl_height = std::min(cl_height, count * itemHeight + ::ContactList::dip(1).px());
            if (count == 0)
                add_height += itemHeight / 2;
        }

        if (regim_ == Logic::MembersWidgetRegim::IGNORE_LIST && chatMembersModel_->rowCount() == 0)
        {
             add_height += itemHeight / 2 + 5;
        }

        new_height = add_height + cl_height;
        //searchWidget_->setFixedWidth(new_width);
        //contactList_->setFixedWidth(new_width);
        //main_dialog_->setFixedWidth(new_width);

        auto main_rect = Ui::get_gui_settings()->get_value(settings_main_window_rect_real, QRect());
        auto main_width = main_rect.width();
        auto main_height = main_rect.height();

        if (_x == default_invalid_coord && _y == default_invalid_coord)
        {
            _x = main_rect.x() + main_width / 2 - new_width /2;
            _y = main_rect.y() + main_height / 2 - new_height / 2;
        }
        return new QRect(_x, _y, new_width, new_height);
    }

	bool SelectContactsWidget::show(int _x, int _y)
	{
        Utils::setWidgetPopup(main_dialog_, true);
        x_ = _x;
        y_ = _y;
		Logic::GetContactListModel()->SetIsWithCheckedBox(true);

        if (Logic::is_delete_members(regim_))
            contactList_->changeTab(ALL);
        else
            Logic::GetCurrentSearchModel(regim_)->searchPatternChanged("");

        auto new_rect = GetSizesAndPosition(_x, _y, is_short_view_);
        main_dialog_->setButtonActive(false);
        connect(this, &SelectContactsWidget::updateMainWidget, main_dialog_, &GeneralDialog::updateParams);

        auto result = main_dialog_->showWithFixedSizes(new_rect->width(), new_rect->height(), new_rect->x(), new_rect->y());
		if (result == QDialog::Accepted)
		{
			if (Logic::GetContactListModel()->GetCheckedContacts().size() == 0)
				result = QDialog::Rejected;
		}
		else
		{
			Logic::GetContactListModel()->ClearChecked();
		}
		Logic::GetContactListModel()->SetIsWithCheckedBox(false);
		searchWidget_->ClearInput();
		return result;
	}

    void SelectContactsWidget::setView(bool)
    {
        chatMembersModel_->is_short_view_ = false;
        searchWidget_->setVisible(true);
        horizontalLineWidget_view_all->setVisible(false);
        view_all->setVisible(false);

        view_all_spacer1->changeSize(1, 0);
        view_all_spacer2->changeSize(1, 0);
        is_short_view_ = false;
    }

    void SelectContactsWidget::onViewAllMembers()
    {
        setView(false);
        chatMembersModel_->load_all_members();
        auto new_rect = GetSizesAndPosition(x_, y_, false);
        emit updateMainWidget(new_rect->width(), new_rect->height(), new_rect->x(), new_rect->y(), true);
    }

    void SelectContactsWidget::UpdateView(bool _is_empty_ignore_list)
    {
        contactList_->setEmptyIgnoreLabelVisible(_is_empty_ignore_list);
        auto new_rect = GetSizesAndPosition(x_, y_, false);
        emit updateMainWidget(new_rect->width(), new_rect->height(), new_rect->x(), new_rect->y(), true);
        main_dialog_->updateGeometry();
    }

    void SelectContactsWidget::UpdateMembers()
    {
        main_dialog_->update();
    }

    bool SelectContactsWidget::ChatNameEditor(QString chat_name, QString* result_chat_name, QWidget* parent, QString _button_text)
    {
        QPalette palette;
        palette.setColor(QPalette::Highlight, "#579e1c");
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::Text, "#282828");

        auto main_widget = new QWidget(parent);
        //Utils::ApplyStyle(main_widget, "background-color: cyan; border-style: solid; border-color: #579e1c; border-width: 2dip;");
        main_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        auto layout = new QVBoxLayout(main_widget);
        layout->setSpacing(0);
        layout->setContentsMargins(Utils::scale_value(24), Utils::scale_value(15), Utils::scale_value(24), 0);

        auto text_edit_ = new TextEditEx(main_widget, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(18), palette, true, true);
        text_edit_->setObjectName("input_edit_control");
        text_edit_->setPlaceholderText(chat_name);
        text_edit_->setPlainText(chat_name);
        text_edit_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        text_edit_->setAutoFillBackground(false);
        text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        text_edit_->setTextInteractionFlags(Qt::TextEditable | Qt::TextEditorInteraction);
        text_edit_->set_catch_enter(true);
        if (platform::is_apple())
            Utils::ApplyStyle(text_edit_, "padding: -4dip; margin: 0;");
        else
            Utils::ApplyStyle(text_edit_, "padding: -2dip; margin: 0;");
        layout->addWidget(text_edit_);

        auto horizontalLineWidget = new QWidget(parent);
        horizontalLineWidget->setFixedHeight(Utils::scale_value(1));
        horizontalLineWidget->setStyleSheet("background-color: #579e1c;");
        layout->addWidget(horizontalLineWidget);

        auto general_dialog = new GeneralDialog(true, true, QT_TRANSLATE_NOOP("groupchat_pages", "Groupchat Name"), _button_text, main_widget, parent, 24);
        connect(text_edit_, SIGNAL(enter()), general_dialog, SLOT(accept()));

        QTextCursor cursor = text_edit_->textCursor();
        cursor.select(QTextCursor::Document);
        text_edit_->setTextCursor(cursor);
        text_edit_->setFrameStyle(QFrame::NoFrame);

        auto main_rect = Ui::get_gui_settings()->get_value(settings_main_window_rect_real, QRect());
        auto main_width = main_rect.width();
        auto main_height = main_rect.height();

        auto new_width = std::max(::ContactList::ItemLength(true, 0.3, ::ContactList::dip(0)).px(), ::ContactList::dip(200).px());
        auto new_height = std::max(::ContactList::ItemLength(false, 0.3, ::ContactList::dip(0)).px(), ::ContactList::dip(200).px());

        auto x = main_rect.x() + main_width / 2 - new_width /2;
        auto y = main_rect.y() + main_height / 2 - new_height / 2;
        SemitransparentWindow window(Ui::get_gui_settings(), parent);
        Utils::setWidgetPopup(general_dialog, true);
        connect(text_edit_, SIGNAL(setSize(int, int)), general_dialog, SLOT(on_resize_child(int, int)), Qt::QueuedConnection);
        text_edit_->setFocus();
        
        auto result = general_dialog->showWithFixedSizes(::ContactList::dip(360).px(), -1, x, y);
        *result_chat_name = text_edit_->toPlainText();
        delete general_dialog;
        return result;
    }
    
}