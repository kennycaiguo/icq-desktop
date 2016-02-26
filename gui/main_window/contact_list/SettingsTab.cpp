#include "stdafx.h"
#include "SettingsTab.h"

#include "../../core_dispatcher.h"
#include "../../gui_settings.h"
#include "../../utils/utils.h"
#include "../../utils/InterConnector.h"
#include "../../utils/gui_coll_helper.h"
#include "../../controls/CustomButton.h"
#include "../../controls/Alert.h"

namespace Ui
{
    class SettingsTab::UI
    {
    private:
        QVBoxLayout *verticalLayout_3;
        QWidget *settings_view;
        QVBoxLayout *verticalLayout_4;
        QLabel *settings_topic_label;
        CustomButton *settings_my_profile_button;
        CustomButton *settings_general_button;
        CustomButton *settings_voice_video_button;
        CustomButton *settings_notifications_button;
        QWidget *widget_2;
        QHBoxLayout *horizontalLayout_6;
        QFrame *line;
        CustomButton *settings_about_button;
        CustomButton *settings_contact_us_button;
        CustomButton *settings_signout_button;
        QSpacerItem *verticalSpacer_2;
        
        friend class SettingsTab;

    public:
        void init(QWidget *p, QLayout *pl)
        {
            settings_topic_label = new QLabel(p);
            Utils::grabTouchWidget(settings_topic_label);
            settings_topic_label->setObjectName(QStringLiteral("settings_topic_label"));
            settings_topic_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            pl->addWidget(settings_topic_label);
            
            settings_my_profile_button = new CustomButton(settings_view, ":/resources/contr_profile_100.png");
            Utils::grabTouchWidget(settings_my_profile_button);
            settings_my_profile_button->setObjectName(QStringLiteral("settings_my_profile_button"));
            settings_my_profile_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_my_profile_button->setMouseTracking(true);
            settings_my_profile_button->setFlat(true);
            settings_my_profile_button->setCheckable(true);
            settings_my_profile_button->setAlign(Qt::AlignLeft);
            settings_my_profile_button->setOffsets(Utils::scale_value(24), 0);
			settings_my_profile_button->setFocusPolicy(Qt::NoFocus);
			settings_my_profile_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_my_profile_button);
            
            settings_general_button = new CustomButton(settings_view, ":/resources/tabs_settings_100.png");
            Utils::grabTouchWidget(settings_general_button);
            settings_general_button->setObjectName(QStringLiteral("settings_general_button"));
            settings_general_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_general_button->setMouseTracking(true);
            settings_general_button->setFlat(true);
            settings_general_button->setCheckable(true);
            settings_general_button->setAlign(Qt::AlignLeft);
            settings_general_button->setOffsets(Utils::scale_value(24), 0);
			settings_general_button->setFocusPolicy(Qt::NoFocus);
			settings_general_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_general_button);
            
            settings_voice_video_button = new CustomButton(settings_view, ":/resources/contr_video_100.png");
            Utils::grabTouchWidget(settings_voice_video_button);
            settings_voice_video_button->setObjectName(QStringLiteral("settings_voice_video_button"));
            settings_voice_video_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_voice_video_button->setMouseTracking(true);
            settings_voice_video_button->setFlat(true);
            settings_voice_video_button->setCheckable(true);
            settings_voice_video_button->setAlign(Qt::AlignLeft);
            settings_voice_video_button->setOffsets(Utils::scale_value(24), 0);
			settings_voice_video_button->setFocusPolicy(Qt::NoFocus);
			settings_voice_video_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_voice_video_button);
            
            settings_notifications_button = new CustomButton(settings_view, ":/resources/contr_notifysettings_100.png");
            Utils::grabTouchWidget(settings_notifications_button);
            settings_notifications_button->setObjectName(QStringLiteral("settings_notifications_button"));
            settings_notifications_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_notifications_button->setMouseTracking(true);
            settings_notifications_button->setFlat(true);
            settings_notifications_button->setCheckable(true);
            settings_notifications_button->setAlign(Qt::AlignLeft);
            settings_notifications_button->setOffsets(Utils::scale_value(24), 0);
			settings_notifications_button->setFocusPolicy(Qt::NoFocus);
			settings_notifications_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_notifications_button);
            
            widget_2 = new QWidget(settings_view);
            Utils::grabTouchWidget(widget_2);
            widget_2->setObjectName(QStringLiteral("widget_2"));
            horizontalLayout_6 = new QHBoxLayout(widget_2);
            horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
            horizontalLayout_6->setContentsMargins(Utils::scale_value(15), Utils::scale_value(15), Utils::scale_value(15), Utils::scale_value(15));
            line = new QFrame(widget_2);
            line->setObjectName(QStringLiteral("line"));
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            horizontalLayout_6->addWidget(line);
            pl->addWidget(widget_2);
            
            settings_about_button = new CustomButton(settings_view, ":/resources/contr_about_100.png");
            Utils::grabTouchWidget(settings_about_button);
            settings_about_button->setObjectName(QStringLiteral("settings_about_button"));
            settings_about_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_about_button->setMouseTracking(true);
            settings_about_button->setFlat(true);
            settings_about_button->setCheckable(true);
            settings_about_button->setAlign(Qt::AlignLeft);
            settings_about_button->setOffsets(Utils::scale_value(24), 0);
			settings_about_button->setFocusPolicy(Qt::NoFocus);
			settings_about_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_about_button);

            settings_contact_us_button = new CustomButton(settings_view, ":/resources/contr_contact_us_100.png");
            Utils::grabTouchWidget(settings_contact_us_button);
            settings_contact_us_button->setObjectName(QStringLiteral("settings_contact_us_button"));
            settings_contact_us_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_contact_us_button->setMouseTracking(true);
            settings_contact_us_button->setFlat(true);
            settings_contact_us_button->setCheckable(true);
            settings_contact_us_button->setAlign(Qt::AlignLeft);
            settings_contact_us_button->setOffsets(Utils::scale_value(24), 0);
            settings_contact_us_button->setFocusPolicy(Qt::NoFocus);
            settings_contact_us_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_contact_us_button);

            settings_signout_button = new CustomButton(settings_view, ":/resources/contr_signout_100.png");
            Utils::grabTouchWidget(settings_signout_button);
            settings_signout_button->setObjectName(QStringLiteral("settings_signout_button"));
            settings_signout_button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
            settings_signout_button->setMouseTracking(true);
            settings_signout_button->setFlat(true);
            settings_signout_button->setCheckable(true);
            settings_signout_button->setAlign(Qt::AlignLeft);
            settings_signout_button->setOffsets(Utils::scale_value(24), 0);
			settings_signout_button->setFocusPolicy(Qt::NoFocus);
			settings_signout_button->setCursor(QCursor(Qt::PointingHandCursor));
            pl->addWidget(settings_signout_button);
            
            settings_topic_label->setText(QT_TRANSLATE_NOOP("settings_pages","Settings"));
            settings_my_profile_button->setText(QT_TRANSLATE_NOOP("settings_pages", "My profile"));
            settings_general_button->setText(QT_TRANSLATE_NOOP("settings_pages", "General"));
            settings_voice_video_button->setText(QT_TRANSLATE_NOOP("settings_pages", "Voice and video"));
            settings_notifications_button->setText(QT_TRANSLATE_NOOP("settings_pages", "Notifications"));
            settings_about_button->setText(QT_TRANSLATE_NOOP("settings_pages", "About"));
            settings_contact_us_button->setText(QT_TRANSLATE_NOOP("settings_pages", "Contact us"));
            settings_signout_button->setText(QT_TRANSLATE_NOOP("settings_pages", "Change account"));
        }
        
    };
    
    SettingsTab::SettingsTab(QWidget* _parent):
        QWidget(_parent),
        Ui_(new UI()),
        CurrentSettingsItem_(SETTINGS_NONE),
        logouter_(Alert::create())
    {
        setStyleSheet(Utils::LoadStyle(":/main_window/contact_list/SettingsTab.qss", Utils::get_scale_coefficient(), true));
        setObjectName(QStringLiteral("settings_main"));

        auto scroll_area = new QScrollArea(this);
        scroll_area->setObjectName(QStringLiteral("scroll_area"));
        scroll_area->setWidgetResizable(true);
        scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        Utils::grabTouchWidget(scroll_area->viewport(), true);
        
        auto scroll_area_content = new QWidget(scroll_area);
        scroll_area_content->setObjectName(QStringLiteral("settings_view"));
        scroll_area_content->setGeometry(QRect(0, 0, Utils::scale_value(800), Utils::scale_value(600)));
        Utils::grabTouchWidget(scroll_area_content);
        
        auto scroll_area_content_layout = new QVBoxLayout(scroll_area_content);
        scroll_area_content_layout->setSpacing(0);
        scroll_area_content_layout->setAlignment(Qt::AlignTop);
        scroll_area_content_layout->setContentsMargins(Utils::scale_value(0), 0, 0, Utils::scale_value(0));
        
        scroll_area->setWidget(scroll_area_content);
        
        auto layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(scroll_area);
        
        Ui_->init(scroll_area_content, scroll_area_content_layout);
        
        logouter_->setText(QT_TRANSLATE_NOOP("popup_window","Are you sure you want to sign out?"));
		logouter_->addButton(QT_TRANSLATE_NOOP("popup_window", "Cancel"), false, [this](QPushButton *)
		{
			logouter_->hide();
			updateSettingsState();
		});
		logouter_->addButton(QT_TRANSLATE_NOOP("popup_window", "Yes"), true, [this](QPushButton *)
        {
            logouter_->hide();
            if (CurrentSettingsItem_ == SETTINGS_PROFILE)
                emit Utils::InterConnector::instance().profileSettingsBack();
            else
                emit Utils::InterConnector::instance().generalSettingsBack();
            CurrentSettingsItem_ = SETTINGS_NONE;
            updateSettingsState();
            get_gui_settings()->set_value(settings_feedback_email, QString(""));
            GetDispatcher()->post_message_to_core("logout", nullptr);
        });
                
        connect(Ui_->settings_my_profile_button, SIGNAL(toggled(bool)), this, SLOT(settingsProfileClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_general_button, SIGNAL(toggled(bool)), this, SLOT(settingsGeneralClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_voice_video_button, SIGNAL(toggled(bool)), this, SLOT(settingsVoiceVideoClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_notifications_button, SIGNAL(toggled(bool)), this, SLOT(settingsNotificationsClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_about_button, SIGNAL(toggled(bool)), this, SLOT(settingsAboutClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_contact_us_button, SIGNAL(toggled(bool)), this, SLOT(settingsContactUsClicked()), Qt::QueuedConnection);
        connect(Ui_->settings_signout_button, SIGNAL(toggled(bool)), this, SLOT(settingsSignoutClicked()), Qt::QueuedConnection);

#ifdef STRIP_VOIP
        Ui_->settings_voice_video_button->hide();
#endif //STRIP_VOIP
    }
    
    SettingsTab::~SettingsTab() throw()
    {
        //
    }
    
    void SettingsTab::cleanSelection()
    {
        emit Utils::InterConnector::instance().popPagesToRoot();

        CurrentSettingsItem_ = SETTINGS_NONE;
        updateSettingsState();
    }
    
    void SettingsTab::settingsProfileClicked()
    {
        if (CurrentSettingsItem_ != SETTINGS_PROFILE && CurrentSettingsItem_ != SETTINGS_NONE)
            emit Utils::InterConnector::instance().generalSettingsBack();

        CurrentSettingsItem_ = SETTINGS_PROFILE;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().profileSettingsShow("");
    }
    
    void SettingsTab::settingsGeneralClicked()
    {
        if (CurrentSettingsItem_ == SETTINGS_PROFILE)
            emit Utils::InterConnector::instance().profileSettingsBack();

        CurrentSettingsItem_ = SETTINGS_GENERAL;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().generalSettingsShow(Utils::InterConnector::CommonSettingsType_General);
    }
    
    void SettingsTab::settingsVoiceVideoClicked()
    {
        if (CurrentSettingsItem_ == SETTINGS_PROFILE)
            emit Utils::InterConnector::instance().profileSettingsBack();

        CurrentSettingsItem_ = SETTINGS_VOICE_VIDEO;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().generalSettingsShow(Utils::InterConnector::CommonSettingsType_VoiceVideo);
    }
    
    void SettingsTab::settingsNotificationsClicked()
    {
        if (CurrentSettingsItem_ == SETTINGS_PROFILE)
            emit Utils::InterConnector::instance().profileSettingsBack();

        CurrentSettingsItem_ = SETTINGS_NOTIFICATIONS;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().generalSettingsShow(Utils::InterConnector::CommonSettingsType_Notifications);
    }
    
    void SettingsTab::settingsAboutClicked()
    {
        if (CurrentSettingsItem_ == SETTINGS_PROFILE)
            emit Utils::InterConnector::instance().profileSettingsBack();

        CurrentSettingsItem_ = SETTINGS_ABOUT;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().generalSettingsShow(Utils::InterConnector::CommonSettingsType_About);
    }

    void SettingsTab::settingsContactUsClicked()
    {
        if (CurrentSettingsItem_ == SETTINGS_PROFILE)
            emit Utils::InterConnector::instance().profileSettingsBack();
        
        CurrentSettingsItem_ = SETTINGS_CONTACT_US;
        updateSettingsState();
        
        emit Utils::InterConnector::instance().generalSettingsShow(Utils::InterConnector::CommonSettingsType_ContactUs);
    }

    void SettingsTab::settingsSignoutClicked()
    {
        updateSettingsState();
        logouter_->show();
    }
    
    void SettingsTab::updateSettingsState()
    {
        Ui_->settings_my_profile_button->blockSignals(true);
        Ui_->settings_general_button->blockSignals(true);
        Ui_->settings_voice_video_button->blockSignals(true);
        Ui_->settings_notifications_button->blockSignals(true);
        Ui_->settings_about_button->blockSignals(true);
        Ui_->settings_contact_us_button->blockSignals(true);
        Ui_->settings_signout_button->blockSignals(true);
        
        Ui_->settings_my_profile_button->setChecked(CurrentSettingsItem_ == SETTINGS_PROFILE);
        Ui_->settings_general_button->setChecked(CurrentSettingsItem_ == SETTINGS_GENERAL);
        Ui_->settings_voice_video_button->setChecked(CurrentSettingsItem_ == SETTINGS_VOICE_VIDEO);
        Ui_->settings_notifications_button->setChecked(CurrentSettingsItem_ == SETTINGS_NOTIFICATIONS);
        Ui_->settings_about_button->setChecked(CurrentSettingsItem_ == SETTINGS_ABOUT);
        Ui_->settings_contact_us_button->setChecked(CurrentSettingsItem_ == SETTINGS_CONTACT_US);
        Ui_->settings_signout_button->setChecked(false);
        
        Ui_->settings_my_profile_button->blockSignals(false);
        Ui_->settings_general_button->blockSignals(false);
        Ui_->settings_voice_video_button->blockSignals(false);
        Ui_->settings_notifications_button->blockSignals(false);
        Ui_->settings_about_button->blockSignals(false);
        Ui_->settings_contact_us_button->blockSignals(false);
        Ui_->settings_signout_button->blockSignals(false);
    }

}
