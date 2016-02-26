#ifndef __VIDEO_WINDOW_H__
#define __VIDEO_WINDOW_H__

#include "VideoPanel.h"
#include "VideoPanelHeader.h"
#include "DetachedVideoWnd.h"
#include "VoipSysPanelHeader.h"

namespace voip_manager{
    struct FrameSize;
}

namespace Ui {
    class videoWindow;
    class DetachedVideoWindow;

    namespace video_window {
        class ResizeEventFilter : public QObject {
            Q_OBJECT

        public:
            ResizeEventFilter(std::vector<QWidget*>& top_panels, std::vector<QWidget*>& bottom_panels, QObject* parent);

        protected:
            bool eventFilter(QObject* obj, QEvent* event);

        private:
            std::vector<QWidget*> _top_panels;
            std::vector<QWidget*> _bottom_panels;
        };
    }

    class VideoWindow : public AspectRatioResizebleWnd {
        Q_OBJECT

    protected:
        void showEvent(QShowEvent*) override;
        void hideEvent(QHideEvent*) override;
        void resizeEvent(QResizeEvent* e) override;
        void closeEvent(QCloseEvent *) override;
        void paintEvent(QPaintEvent *e) override;
        void changeEvent(QEvent *) override;
        void escPressed() override;

	private:
        quintptr getContentWinId() override;

    private Q_SLOTS:
        void _check_overlap();
		void _check_panels_vis();
        void onVoipMouseTapped(quintptr, const std::string& tap_type);
        void onVoipCallNameChanged(const std::vector<voip_manager::Contact>&);
		void onVoipCallTimeChanged(unsigned sec_elapsed, bool have_call);
        void onVoipCallDestroyed(const voip_manager::ContactEx& contact_ex);
        void onVoipMediaRemoteVideo(bool enabled);
        void _escPressed();

        void onPanelClickedClose();
        void onPanelClickedMinimize();
        void onPanelClickedMaximize();

		void onPanelMouseEnter();
		void onPanelMouseLeave();
		void onPanelFullscreenClicked();

    public:
        VideoWindow();
        ~VideoWindow();

    private:
        std::unique_ptr<VideoPanel>          video_panel_;
        std::unique_ptr<VideoPanelHeader>    video_panel_header_;
        std::unique_ptr<VoipSysPanelHeader>  video_panel_header_with_avatars_;
        std::unique_ptr<DetachedVideoWindow> detached_wnd_;
        video_window::ResizeEventFilter*     event_filter_;
        QTimer check_overlapped_timer_;
		QTimer show_panel_timer_;
        bool have_remote_video_;
		UIEffects* video_panel_effect_;
		UIEffects* video_panel_header_effect_;
        UIEffects* video_panel_header_effect_with_avatars_;
        QVBoxLayout *vertical_layout_;
        QVBoxLayout *vertical_layout_2_;
        QWidget *widget_;
    };
}

#endif//__VIDEO_WINDOW_H__