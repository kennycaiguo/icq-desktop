#pragma once

namespace Ui
{
    class Alert;
    
    enum CurrentSettingsItem
    {
        SETTINGS_NONE = -1,
        SETTINGS_PROFILE = 0,
        SETTINGS_GENERAL,
        SETTINGS_VOICE_VIDEO,
        SETTINGS_NOTIFICATIONS,
        SETTINGS_ABOUT,
        SETTINGS_CONTACT_US,
        
        SETTINGS_MAX = SETTINGS_CONTACT_US + 1,
    };
    
    class SettingsTab : public QWidget
    {
        class UI;

        Q_OBJECT
        
    private Q_SLOTS:
        void settingsProfileClicked();
        void settingsGeneralClicked();
        void settingsVoiceVideoClicked();
        void settingsNotificationsClicked();
        void settingsAboutClicked();
        void settingsContactUsClicked();
        void settingsSignoutClicked();

    private:
        std::unique_ptr< UI > Ui_;
        unsigned CurrentSettingsItem_;
        std::unique_ptr<Alert> logouter_;

    public:
        SettingsTab(QWidget* _parent);
        ~SettingsTab() throw();
        
        void cleanSelection();
        
    private:
        void updateSettingsState();
    };
}