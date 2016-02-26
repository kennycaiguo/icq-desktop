#include "stdafx.h"
#include "VideoWindow.h"
#include "DetachedVideoWnd.h"
#include "../../core/Voip/VoipManagerDefines.h"

#include "../core_dispatcher.h"
#include "../utils/utils.h"
#include "../cache/avatars/AvatarStorage.h"
#include "../main_window/contact_list/ContactListModel.h"

extern std::string getFotmatedTime(unsigned ts);
namespace {
	enum { kPreviewBorderOffset = 12 };
    enum { kMinimumW = 328, kMinimumH = 164 };
    enum { kDefaultW = 640, kDefaultH = 480 };
    enum { kAnimationDefDuration = 500 };

#ifdef _WIN32
    bool windowIsOverlapped(WId wnd, quintptr* exclude, int size) {
        HWND target = (HWND)wnd;
        if (!::IsWindowVisible(target)) {
            return false;
        }

        RECT r;
        ::GetWindowRect(target, &r);

        const int overlapDepthPts = 90;
        typedef std::vector<POINT> ptList;
        ptList pts;

        int ptsNumY = (r.bottom - r.top)/overlapDepthPts;
        int ptsNumX = (r.right - r.left)/overlapDepthPts;
        for( int j=0; j<ptsNumY; ++j ) 
        {
            for( int i=0; i<ptsNumX; ++i ) 
            {
                int ptX = r.left + overlapDepthPts*i;
                int ptY = r.top	 + overlapDepthPts*j;

                POINT pt = { ptX, ptY };
                pts.push_back(pt);
            }
        }

        int ptsCounter = 0;
        for (ptList::const_iterator it = pts.begin(); it != pts.end(); ++it) {
            const HWND top = ::WindowFromPoint( *it );

            bool isMyWnd = top == target;
            for (int i = 0; i < size; i++) {
                isMyWnd |= top == (HWND)exclude[i];
            }

            if (!isMyWnd) {
                ++ptsCounter;
            }
        }

        return (ptsCounter * 10) >= int(pts.size() * 4); // 40 % overlapping
    }
#else
#endif
}

Ui::video_window::ResizeEventFilter::ResizeEventFilter(std::vector<QWidget*>& top_panels, std::vector<QWidget*>& bottom_panels, QObject* parent) 
: QObject(parent) 
, _top_panels(top_panels)
, _bottom_panels(bottom_panels) {

}

bool Ui::video_window::ResizeEventFilter::eventFilter(QObject* obj, QEvent* e) {
    if (e->type() == QEvent::Resize || 
        e->type() == QEvent::Move || 
        e->type() == QEvent::WindowActivate || 
        e->type() == QEvent::NonClientAreaMouseButtonPress ||
        e->type() == QEvent::ZOrderChange) {
        QWidget* parent = qobject_cast<QWidget*>(obj);
        const QRect rect = parent->rect();
        const QPoint lt = parent->mapToGlobal(rect.topLeft());
        const QRect rc(lt.x(), lt.y(), rect.width(), rect.height());

        bool needToRaise = parent->isActiveWindow();
        for (unsigned ix = 0; ix < _bottom_panels.size(); ix++) {
            QWidget* panel = _bottom_panels[ix];
            if (!panel) {
                continue;
            }

            needToRaise |= panel->isActiveWindow();
        }

        for (unsigned ix = 0; ix < _top_panels.size(); ix++) {
            QWidget* panel = _top_panels[ix];
            if (!panel) {
                continue;
            }

            needToRaise |= panel->isActiveWindow();
        }


        for (unsigned ix = 0; ix < _bottom_panels.size(); ix++) {
            QWidget* panel = _bottom_panels[ix];
            if (!panel) {
                continue;
            }

            panel->move(rc.x(), rc.y() + rc.height() - panel->rect().height());
            panel->setFixedWidth(rc.width());

            if (needToRaise)
                panel->raise();
        }

        for (unsigned ix = 0; ix < _top_panels.size(); ix++) {
            QWidget* panel = _top_panels[ix];
            if (!panel) {
                continue;
            }

            panel->move(rc.x(), rc.y());
            panel->setFixedWidth(rc.width());

            if (needToRaise)
                panel->raise();
        }
    }
    return QObject::eventFilter(obj, e);
}


Ui::VideoWindow::VideoWindow()
: AspectRatioResizebleWnd()
, check_overlapped_timer_(this)
, show_panel_timer_(this)
, have_remote_video_(false)
, video_panel_header_with_avatars_(new VoipSysPanelHeader(this))
, detached_wnd_(new DetachedVideoWindow(this))
, video_panel_(new(std::nothrow) VideoPanel(NULL, this))
, video_panel_header_(new(std::nothrow) VideoPanelHeader(this, kVPH_ShowName | kVPH_ShowTime | kVPH_ShowMin | kVPH_ShowClose)) {
 
    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("videoWindow"));
    this->resize(368, 226);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    this->setSizePolicy(sizePolicy);
    this->setMinimumSize(QSize(0, 0));
    this->setProperty("VideoWindowCommon", QVariant(true));
    vertical_layout_ = new QVBoxLayout(this);
    vertical_layout_->setSpacing(0);
    vertical_layout_->setObjectName(QStringLiteral("verticalLayout"));
    vertical_layout_->setContentsMargins(0, 0, 0, 0);
    widget_ = new QWidget(this);
    widget_->setObjectName(QStringLiteral("widget"));
    vertical_layout_2_ = new QVBoxLayout(widget_);
    vertical_layout_2_->setObjectName(QStringLiteral("verticalLayout_2"));
    
    vertical_layout_->addWidget(widget_);
    QMetaObject::connectSlotsByName(this);
    
    

    QIcon icon(":/resources/main_window/appicon.ico");
    setWindowIcon(icon);

    widget_->setAttribute(Qt::WA_UpdatesDisabled);

	video_panel_effect_ = new UIEffects(*video_panel_.get());
	video_panel_header_effect_ = new UIEffects(*video_panel_header_.get());
    video_panel_header_effect_with_avatars_ = new UIEffects(*video_panel_header_with_avatars_.get());

    std::vector<QWidget*> topPanels;
    topPanels.push_back(video_panel_header_.get());
    topPanels.push_back(video_panel_header_with_avatars_.get());

    std::vector<QWidget*> bottomPanels;
    bottomPanels.push_back(video_panel_.get());

	event_filter_ = new video_window::ResizeEventFilter(topPanels, bottomPanels, this);
    installEventFilter(event_filter_);

    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    setProperty("VideoWindow", true);

    if (video_panel_) {
        video_panel_->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
        video_panel_->setAttribute(Qt::WA_NoSystemBackground, true);
        video_panel_->setAttribute(Qt::WA_TranslucentBackground, true);  
		video_panel_->setFullscreenMode(isInFullscreen());

        connect(video_panel_.get(), SIGNAL(onMouseEnter()), this, SLOT(onPanelMouseEnter()), Qt::QueuedConnection);
		connect(video_panel_.get(), SIGNAL(onMouseLeave()), this, SLOT(onPanelMouseLeave()), Qt::QueuedConnection);
		connect(video_panel_.get(), SIGNAL(onFullscreenClicked()), this, SLOT(onPanelFullscreenClicked()), Qt::QueuedConnection);
        connect(video_panel_.get(), SIGNAL(onkeyEscPressed()), this, SLOT(_escPressed()), Qt::QueuedConnection);
    }

    if (!!video_panel_header_) {
        video_panel_header_->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
        video_panel_header_->setAttribute(Qt::WA_NoSystemBackground, true);
        video_panel_header_->setAttribute(Qt::WA_TranslucentBackground, true);  

        connect(video_panel_header_.get(), SIGNAL(onMouseEnter()), this, SLOT(onPanelMouseEnter()), Qt::QueuedConnection);
		connect(video_panel_header_.get(), SIGNAL(onMouseLeave()), this, SLOT(onPanelMouseLeave()), Qt::QueuedConnection);

        connect(video_panel_header_.get(), SIGNAL(onClose()), this, SLOT(onPanelClickedClose()), Qt::QueuedConnection);
        connect(video_panel_header_.get(), SIGNAL(onMinimize()), this, SLOT(onPanelClickedMinimize()), Qt::QueuedConnection);
        connect(video_panel_header_.get(), SIGNAL(onMaximize()), this, SLOT(onPanelClickedMaximize()), Qt::QueuedConnection);
        connect(video_panel_header_.get(), SIGNAL(onkeyEscPressed()), this, SLOT(_escPressed()), Qt::QueuedConnection);
    }

    if (!!video_panel_header_with_avatars_) {
        video_panel_header_with_avatars_->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
        video_panel_header_with_avatars_->setAttribute(Qt::WA_NoSystemBackground, true);
        video_panel_header_with_avatars_->setAttribute(Qt::WA_TranslucentBackground, true);  
        video_panel_header_with_avatars_->setTitle("");
        video_panel_header_with_avatars_->setStatus(QT_TRANSLATE_NOOP("voip_pages", "Outgoing call").toUtf8());

        connect(video_panel_header_with_avatars_.get(), SIGNAL(onMouseEnter()), this, SLOT(onPanelMouseEnter()), Qt::QueuedConnection);
		connect(video_panel_header_with_avatars_.get(), SIGNAL(onMouseLeave()), this, SLOT(onPanelMouseLeave()), Qt::QueuedConnection);
        connect(video_panel_header_with_avatars_.get(), SIGNAL(onkeyEscPressed()), this, SLOT(_escPressed()), Qt::QueuedConnection);
    }

    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipCallTimeChanged(unsigned,bool)), this, SLOT(onVoipCallTimeChanged(unsigned,bool)), Qt::DirectConnection);
    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipCallNameChanged(const std::vector<voip_manager::Contact>&)), this, SLOT(onVoipCallNameChanged(const std::vector<voip_manager::Contact>&)), Qt::DirectConnection);
    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipMouseTapped(quintptr,const std::string&)), this, SLOT(onVoipMouseTapped(quintptr,const std::string&)), Qt::DirectConnection);
    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipCallDestroyed(const voip_manager::ContactEx&)), this, SLOT(onVoipCallDestroyed(const voip_manager::ContactEx&)), Qt::DirectConnection);
    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipMediaRemoteVideo(bool)), this, SLOT(onVoipMediaRemoteVideo(bool)), Qt::DirectConnection);
    
    connect(&check_overlapped_timer_, SIGNAL(timeout()), this, SLOT(_check_overlap()), Qt::QueuedConnection);
    check_overlapped_timer_.setInterval(1000);
    check_overlapped_timer_.start();

	connect(&show_panel_timer_, SIGNAL(timeout()), this, SLOT(_check_panels_vis()), Qt::QueuedConnection);

#ifdef _DEBUG
	show_panel_timer_.setInterval(15000);
#else
	show_panel_timer_.setInterval(1500);
#endif

    detached_wnd_->hide();
    setMouseTracking(true);

    setMinimumSize(Utils::scale_value(kMinimumW), Utils::scale_value(kMinimumH));
    resize(Utils::scale_value(kDefaultW), Utils::scale_value(kDefaultH));
}

Ui::VideoWindow::~VideoWindow() {
    check_overlapped_timer_.stop();

	removeEventFilter(event_filter_);
	delete event_filter_;
}

quintptr Ui::VideoWindow::getContentWinId() {
    return (quintptr)widget_->winId();
}

void Ui::VideoWindow::onPanelMouseEnter() {
	show_panel_timer_.stop();
	video_panel_effect_->fadeIn(kAnimationDefDuration);
	video_panel_header_effect_->fadeIn(kAnimationDefDuration);
    video_panel_header_effect_with_avatars_->fadeIn(kAnimationDefDuration);
#ifdef _WIN32
	Ui::GetDispatcher()->getVoipController().setWindowOffsets(
		(quintptr)widget_->winId(),
		Utils::scale_value(kPreviewBorderOffset),
		Utils::scale_value(kPreviewBorderOffset), 
		Utils::scale_value(kPreviewBorderOffset), 
		video_panel_->geometry().height() + Utils::scale_value(kPreviewBorderOffset)
		);
#endif
}

void Ui::VideoWindow::onVoipMediaRemoteVideo(bool enabled) {
    have_remote_video_ = enabled;

    if (video_panel_header_->isVisible() || video_panel_header_with_avatars_->isVisible()) {
        if (have_remote_video_) {
            video_panel_header_->show();
            video_panel_header_with_avatars_->hide();
        } else {
            video_panel_header_->hide();
            video_panel_header_with_avatars_->show();
        }
    }

    if (have_remote_video_) {
        show_panel_timer_.start();
        useAspect();
    } else {
        unuseAspect();
        video_panel_effect_->fadeIn(kAnimationDefDuration);
        video_panel_header_effect_->fadeIn(kAnimationDefDuration);
        video_panel_header_effect_with_avatars_->fadeIn(kAnimationDefDuration);

#ifdef _WIN32
        Ui::GetDispatcher()->getVoipController().setWindowOffsets(
            (quintptr)widget_->winId(),
            Utils::scale_value(kPreviewBorderOffset),
            Utils::scale_value(kPreviewBorderOffset), 
            Utils::scale_value(kPreviewBorderOffset), 
            video_panel_->geometry().height() + Utils::scale_value(kPreviewBorderOffset)
            );
#endif
    }
}

void Ui::VideoWindow::onPanelMouseLeave() {
    if (have_remote_video_) {
        show_panel_timer_.start();
    }
}

void Ui::VideoWindow::_check_overlap() {
    if (!isVisible()) {
        return;
    }

#ifdef _WIN32
    quintptr friendlyWnds[] = { 
        !!video_panel_ ? video_panel_->winId() : NULL,
        !!video_panel_header_ ? video_panel_header_->winId() : NULL,
        !!video_panel_header_with_avatars_ ? video_panel_header_with_avatars_->winId() : NULL
    };
    if (windowIsOverlapped(widget_->winId(), friendlyWnds, sizeof(friendlyWnds) / sizeof(friendlyWnds[0])) && !detached_wnd_->closedManualy()) {
        QApplication::alert(this);
		detached_wnd_->showNormal();
    } else {
        detached_wnd_->hide();
    }
#else
    
#endif
}

void Ui::VideoWindow::_check_panels_vis() {
	show_panel_timer_.stop();
	video_panel_effect_->fadeOut(kAnimationDefDuration);
    video_panel_header_effect_->fadeOut(kAnimationDefDuration);
    video_panel_header_effect_with_avatars_->fadeOut(kAnimationDefDuration);
#ifdef _WIN32
	Ui::GetDispatcher()->getVoipController().setWindowOffsets(
		(quintptr)widget_->winId(), 
		Utils::scale_value(kPreviewBorderOffset), 
		Utils::scale_value(kPreviewBorderOffset), 
		Utils::scale_value(kPreviewBorderOffset), 
		Utils::scale_value(kPreviewBorderOffset)
		);
#endif
}

void Ui::VideoWindow::onVoipCallTimeChanged(unsigned sec_elapsed, bool have_call) {
    video_panel_header_->setTime(sec_elapsed, have_call);

    if (have_call) {
        video_panel_header_with_avatars_->setStatus(getFotmatedTime(sec_elapsed).c_str());
    } else {
        video_panel_header_with_avatars_->setStatus(QT_TRANSLATE_NOOP("voip_pages", "Outgoing call").toUtf8());
    }
}

void Ui::VideoWindow::onPanelClickedMinimize() {
    showMinimized();
}

void Ui::VideoWindow::onPanelClickedMaximize() {
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
    video_panel_->setFullscreenMode(isInFullscreen());
}

void Ui::VideoWindow::onPanelClickedClose() {
#ifdef _WIN32
	Ui::GetDispatcher()->getVoipController().setHangup();
#endif
}

void Ui::VideoWindow::hideEvent(QHideEvent* ev) {
    video_panel_->hide();
    video_panel_header_->hide();
    video_panel_header_with_avatars_->hide();

    QWidget::hideEvent(ev);
    detached_wnd_->hide();
#ifdef _WIN32
    Ui::GetDispatcher()->getVoipController().setWindowRemove((quintptr)widget_->winId());
#endif
}

void Ui::VideoWindow::showEvent(QShowEvent* ev) {
    video_panel_->show();
    if (have_remote_video_) {
        video_panel_header_->show();
        video_panel_header_with_avatars_->hide();
    } else {
        video_panel_header_->hide();
        video_panel_header_with_avatars_->show();
    }

    QWidget::showEvent(ev);

#ifdef _WIN32
    show_panel_timer_.stop();
    video_panel_effect_->fadeIn(kAnimationDefDuration);
    video_panel_header_effect_->fadeIn(kAnimationDefDuration);
    video_panel_header_effect_with_avatars_->fadeIn(kAnimationDefDuration);

	Ui::GetDispatcher()->getVoipController().setWindowOffsets(
		(quintptr)widget_->winId(),
		Utils::scale_value(kPreviewBorderOffset),
		Utils::scale_value(kPreviewBorderOffset), 
		Utils::scale_value(kPreviewBorderOffset), 
		video_panel_->geometry().height() + Utils::scale_value(kPreviewBorderOffset)
		);

    if (have_remote_video_) {
        show_panel_timer_.start();
    }

    Ui::GetDispatcher()->getVoipController().setWindowAdd((quintptr)widget_->winId(), true, false, video_panel_->geometry().height() + Utils::scale_value(5));

    detached_wnd_->hide();
#else
    
#endif

    showNormal();
	activateWindow();
    video_panel_->setFullscreenMode(isInFullscreen());
} 

void Ui::VideoWindow::onVoipMouseTapped(quintptr hwnd, const std::string& tap_type) {
#ifdef _WIN32
	const bool dbl_tap = tap_type == "double";
	const bool over = tap_type == "over";

    if (detached_wnd_->get_video_frame_id() == hwnd) {
        if (dbl_tap) {
            raise();
        }
    } else if ((quintptr)widget_->winId() == hwnd) {
        if (dbl_tap) {
			switchFullscreen();
            video_panel_->setFullscreenMode(isInFullscreen());
        } else if (over) {
			show_panel_timer_.stop();

			video_panel_effect_->fadeIn(kAnimationDefDuration);
			video_panel_header_effect_->fadeIn(kAnimationDefDuration);
            video_panel_header_effect_with_avatars_->fadeIn(kAnimationDefDuration);

			Ui::GetDispatcher()->getVoipController().setWindowOffsets(
				(quintptr)widget_->winId(), 
				Utils::scale_value(kPreviewBorderOffset), 
				Utils::scale_value(kPreviewBorderOffset), 
				Utils::scale_value(kPreviewBorderOffset), 
				video_panel_->geometry().height() + Utils::scale_value(kPreviewBorderOffset)
				);

            if (have_remote_video_) {
			    show_panel_timer_.start();
            }
        }
    }
#endif
}

void Ui::VideoWindow::onPanelFullscreenClicked() {
	switchFullscreen();
    video_panel_->setFullscreenMode(isInFullscreen());
}

void Ui::VideoWindow::onVoipCallNameChanged(const std::vector<voip_manager::Contact>& contacts) {
    if(contacts.empty()) {
        return;
    }

    std::vector<std::string> users;
    std::vector<std::string> friendly_names;
    for(unsigned ix = 0; ix < contacts.size(); ix++) {
        users.push_back(contacts[ix].contact);
        
        // F*KN QSTRING...WHO USE IT???????????!!!!!!!!!!!!
#ifdef _WIN32
        std::string n = Logic::GetContactListModel()->getDisplayName(contacts[ix].contact.c_str()).toUtf8();
        friendly_names.push_back(n);
#endif
    }

    video_panel_header_with_avatars_->setAvatars(users);

    auto name = voip_manager::formatCallName(friendly_names, QT_TRANSLATE_NOOP("voip_pages", "and").toUtf8());
    assert(!name.empty());

    video_panel_header_->setCallName(name);
    video_panel_header_with_avatars_->setTitle(name.c_str());
}

void Ui::VideoWindow::paintEvent(QPaintEvent *e)
{
    return QWidget::paintEvent(e);
}

void Ui::VideoWindow::changeEvent(QEvent* e) {
    QWidget::changeEvent(e);

    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            video_panel_->blockSignals(true);
            video_panel_->raise();
            video_panel_->blockSignals(false);

            video_panel_header_with_avatars_->blockSignals(true);
            video_panel_header_with_avatars_->raise();
            video_panel_header_with_avatars_->blockSignals(false);

            video_panel_header_->blockSignals(true);
            video_panel_header_->raise();
            video_panel_header_->blockSignals(false);

            video_panel_->activateWindow();
        }
    }
}

void Ui::VideoWindow::closeEvent(QCloseEvent* e) {
    QWidget::closeEvent(e);
#ifdef _WIN32
	Ui::GetDispatcher()->getVoipController().setHangup();
#endif
}

void Ui::VideoWindow::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    int border_width = Utils::scale_value(2);
    const auto fg = frameGeometry();
    const auto ge = geometry();

    border_width = std::min(ge.top() - fg.top(), 
                   std::min(fg.right() - ge.right(),
                   std::min(fg.bottom() - ge.bottom(),
                   std::min(ge.left() - fg.left(), 
                   border_width))));

    QRegion reg(-border_width, -border_width, ge.width() + 2*border_width, ge.height() + 2*border_width);
    setMask(reg);
}

void Ui::VideoWindow::onVoipCallDestroyed(const voip_manager::ContactEx& contact_ex) {
    if (contact_ex.call_count <= 1) {
        unuseAspect();
        have_remote_video_ = false;
        _escPressed();
        //resize(Utils::scale_value(kDefaultW), Utils::scale_value(kDefaultH));
        video_panel_header_with_avatars_->setStatus(QT_TRANSLATE_NOOP("voip_pages", "Outgoing call").toUtf8());
    }
}

void Ui::VideoWindow::escPressed() {
    _escPressed();
}

void Ui::VideoWindow::_escPressed() {
    if (isInFullscreen()) {
        switchFullscreen();
        video_panel_->setFullscreenMode(isInFullscreen());
    }
}