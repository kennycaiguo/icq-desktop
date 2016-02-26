#pragma once

#include "../contact_list/contact_profile.h"

namespace Ui
{
    class ContactAvatarWidget;
    
    class ProfileSettingsWidget : public QWidget
    { 
        enum ActionButtonState
        {
            EDIT_PROFILE    = 0,
            USER_ACTIONS,
        };
        
        class UI;
        
        Q_OBJECT
        
    private Q_SLOTS:
        void onAvatarLoaded(QString uin);

        void menuStateOnline();
        void menuStateDoNotDisturb();
        void menuStateInvisible();

        void contactAdd();
        void contactIgnore();
        void contactSpam();
        
        void myInfo();

    private:
        std::unique_ptr<UI> Ui_;
        ContactAvatarWidget* avatar_;
        ActionButtonState actionButtonState_;

        QString uin_;

    public:
        ProfileSettingsWidget(QWidget* _parent);
        virtual ~ProfileSettingsWidget();
      
        void updateInterface(const QString &uin);
        
    private:
        void paintEvent(QPaintEvent* event) override;
        bool event(QEvent* event) override;

        void parse(Logic::profile_ptr profile);

        void setFullName(const QString& val);
        void setICQNumber(const QString& val);
        void setPhone(const QString& val, bool forSelf);
        void setFirstName(const QString& val);
        void setLastName(const QString& val);
        void setBirthdate(const QString& val);
        void setGender(const QString& val);
        void setCountry(const QString& val);
        void setCity(const QString& val);
        void setAbout(const QString& val);

        void setStateOnline();
        void setStateOffline();
        void setStateDoNotDisturb();
        void setStateInvisible();
        
        void updateActionButton();
    };
}

