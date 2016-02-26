#include "stdafx.h"
#include "IncomingCallWindow.h"
#include "../core_dispatcher.h"
#include "DetachedVideoWnd.h"
#include "VideoWindow.h"
#include "../../core/Voip/VoipManagerDefines.h"

#include "CallPanelMain.h"
#include "../cache/avatars/AvatarStorage.h"
#include "../utils/utils.h"
#include "../main_window/contact_list/ContactListModel.h"

namespace {
    enum {
        kIncomingCallWndDefH = 285,
        kIncomingCallWndDefW = 390,
    };

#ifdef _DEBUG
    const int ht[kIncomingCallWndDefH > 0 ? 1 : -1] = { 0 }; // kIncomingCallWndDefH must be not null
    const int wt[kIncomingCallWndDefW > 0 ? 1 : -1] = { 0 }; // kIncomingCallWndDefW must be not null
#endif
}

Ui::IncomingCallWindow::IncomingCallWindow(const std::string& account, const std::string& contact)
: QWidget(NULL) 
, contact_(contact)
, account_(account)
, header_(new(std::nothrow) VoipSysPanelHeader(this))
, controls_(new VoipSysPanelControl(this)) {
    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("incomingCallWindow"));
    vertical_layout_ = new QVBoxLayout(this);
    vertical_layout_->setObjectName(QStringLiteral("verticalLayout"));
    QMetaObject::connectSlotsByName(this);
    
    hide();
    QIcon icon(":/resources/main_window/appicon.ico");
    setWindowIcon(icon);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Window);
    setProperty("IncomingCallWindow", true);

    header_->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    header_->setAttribute(Qt::WA_NoSystemBackground, true);
    header_->setAttribute(Qt::WA_TranslucentBackground, true); 

    controls_->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    controls_->setAttribute(Qt::WA_NoSystemBackground, true);
    controls_->setAttribute(Qt::WA_TranslucentBackground, true); 

    std::vector<QWidget*> topPanels;
    topPanels.push_back(header_.get());

    std::vector<QWidget*> bottomPanels;
    bottomPanels.push_back(controls_.get());

	event_filter_ = new video_window::ResizeEventFilter(topPanels, bottomPanels, this);
    installEventFilter(event_filter_);

    connect(controls_.get(), SIGNAL(onDecline()), this, SLOT(onDeclineButtonClicked()), Qt::QueuedConnection);
    connect(controls_.get(), SIGNAL(onVideo()), this, SLOT(onAcceptVideoClicked()),   Qt::QueuedConnection);
    connect(controls_.get(), SIGNAL(onAudio()), this, SLOT(onAcceptAudioClicked()),   Qt::QueuedConnection);

    QObject::connect(&Ui::GetDispatcher()->getVoipController(), SIGNAL(onVoipCallNameChanged(const std::vector<voip_manager::Contact>&)), this, SLOT(onVoipCallNameChanged(const std::vector<voip_manager::Contact>&)), Qt::DirectConnection);

    const QSize defaultSize(Utils::scale_value(kIncomingCallWndDefW), Utils::scale_value(kIncomingCallWndDefH));
    setMinimumSize(defaultSize);
    setMaximumSize(defaultSize);
    resize(defaultSize);

    QDesktopWidget dw;
    const auto screen_rect = dw.screenGeometry(dw.primaryScreen());
    const auto wndSize = defaultSize;
    const auto center  = screen_rect.center();

    const QRect rc(center.x() - wndSize.width()*0.5f, center.y() - wndSize.height()*0.5f, wndSize.width(), wndSize.height());
    setGeometry(rc);
}

Ui::IncomingCallWindow::~IncomingCallWindow() {
    removeEventFilter(event_filter_);
	delete event_filter_;
}

void Ui::IncomingCallWindow::showEvent(QShowEvent* e) {
    header_->show();
    controls_->show();
    QWidget::showEvent(e);
#ifdef _WIN32
    Ui::GetDispatcher()->getVoipController().setWindowAdd((quintptr)winId(), false, true, 0);
#endif
        //auto a = new QPropertyAnimation(this, "size");
        //a->setDuration(1000);

        //a->setStartValue(QSize(0, 0));
        //a->setEndValue(QSize(350 * GuiSettings()->scale_coefficient, 250 * GuiSettings()->scale_coefficient));
        //a->start();
}

void Ui::IncomingCallWindow::hideEvent(QHideEvent* e) {
    header_->hide();
    controls_->hide();
    QWidget::hideEvent(e);
#ifdef _WIN32
    Ui::GetDispatcher()->getVoipController().setWindowRemove((quintptr)winId());
#endif
}

void Ui::IncomingCallWindow::changeEvent(QEvent* e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            header_->blockSignals(true);
            header_->raise();
            header_->blockSignals(false);

            controls_->blockSignals(true);
            controls_->raise();
            controls_->blockSignals(false);

            controls_->activateWindow();
        }
    }
}

void Ui::IncomingCallWindow::onVoipCallNameChanged(const std::vector<voip_manager::Contact>& contacts) {
    if(contacts.empty()) {
        return;
    }

    if (contacts[0].account == account_ && contacts[0].contact == contact_) {
        std::vector<std::string> users;
        std::vector<std::string> friendly_names;
        for(unsigned ix = 0; ix < contacts.size(); ix++) {
            users.push_back(contacts[ix].contact);
#ifdef _WIN32
            std::string n = Logic::GetContactListModel()->getDisplayName(contacts[ix].contact.c_str()).toUtf8();
            friendly_names.push_back(n);
#endif
        }

        header_->setAvatars(users);

        auto name = voip_manager::formatCallName(friendly_names, QT_TRANSLATE_NOOP("voip_pages", "and").toUtf8());
        assert(!name.empty());

        header_->setTitle(name.c_str());
        header_->setStatus(QT_TRANSLATE_NOOP("voip_pages", "Incoming call").toUtf8());
    }
}

void Ui::IncomingCallWindow::onAcceptVideoClicked() {
    assert(!contact_.empty());
    if (!contact_.empty()) {
#ifdef _WIN32
		Ui::GetDispatcher()->getVoipController().setAcceptV(contact_.c_str());
#endif
    }
}

void Ui::IncomingCallWindow::onAcceptAudioClicked() {
    assert(!contact_.empty());
    if (!contact_.empty()) {
#ifdef _WIN32
		Ui::GetDispatcher()->getVoipController().setAcceptA(contact_.c_str());
#endif
    }
}


void Ui::IncomingCallWindow::onDeclineButtonClicked() {
    assert(!contact_.empty());
    if (!contact_.empty()) {
#ifdef _WIN32
		Ui::GetDispatcher()->getVoipController().setDecline(contact_.c_str(), false);
#endif
    }
}

//void Ui::IncomingCallWindow::onVoipCallCreated(const voip_manager::ContactEx& contact_ex) {
//    assert(false);
//}
