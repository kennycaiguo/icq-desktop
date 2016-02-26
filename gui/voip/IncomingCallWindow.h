#ifndef __INCOMING_CALL_WINDOW_H__
#define __INCOMING_CALL_WINDOW_H__

#include "VoipSysPanelHeader.h"

namespace Ui {
    class incomingCallWindow;

    namespace video_window {
        class ResizeEventFilter;
    }

    class IncomingCallWindow : public QWidget {
        std::string contact_;
        std::string account_;

        Q_OBJECT

    private Q_SLOTS:
        void onVoipCallNameChanged(const std::vector<voip_manager::Contact>&);
        //void onVoipCallCreated(const voip_manager::ContactEx& contact_ex);

        void onDeclineButtonClicked();
        void onAcceptVideoClicked();
        void onAcceptAudioClicked();

    public:
        IncomingCallWindow(const std::string& account, const std::string& contact);
        ~IncomingCallWindow();

    private:
        std::unique_ptr<VoipSysPanelHeader> header_;
        std::unique_ptr<VoipSysPanelControl> controls_;
        video_window::ResizeEventFilter* event_filter_;
        QVBoxLayout *vertical_layout_;

        void showEvent(QShowEvent*) override;
        void hideEvent(QHideEvent*) override;
        void changeEvent(QEvent *) override;
    };
}

#endif//__INCOMING_CALL_WINDOW_H__