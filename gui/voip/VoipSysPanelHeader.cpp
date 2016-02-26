#include "stdafx.h"
#include "VoipSysPanelHeader.h"
#include "../utils/utils.h"
#ifdef __APPLE__
#   include <QtWidgets/qtoolbutton.h>
#else
#   include <QToolButton>
#   include <QStyleOptionToolButton>
#endif // __APPLE__

#define VOIP_VIDEO_PANEL_BTN_OFFSET (Utils::scale_value(40))
#include "../../installer/utils/styles.h"

Ui::VoipSysPanelControl::VoipSysPanelControl(QWidget* parent)
: QWidget(NULL)
, _rootWidget(NULL)
, _parent(parent) {
    setProperty("VoipSysPanelControls", true);
    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);  

    QVBoxLayout* root_layout = new QVBoxLayout();
    root_layout->setContentsMargins(0, 0, 0, 0);
    root_layout->setSpacing(0);
    root_layout->setAlignment(Qt::AlignVCenter);
    setLayout(root_layout);

    _rootWidget = new QWidget(this);
    _rootWidget->setContentsMargins(0, 0, 0, 0);
    _rootWidget->setProperty("VoipSysPanelControls", true);
    layout()->addWidget(_rootWidget);
    
    QHBoxLayout* layoutTarget = new QHBoxLayout();
    layoutTarget->setContentsMargins(0, 0, 0, 0);
    layoutTarget->setSpacing(0);
    layoutTarget->setAlignment(Qt::AlignVCenter);
    _rootWidget->setLayout(layoutTarget);

    QFont font = QApplication::font();
    font.setStyleStrategy(QFont::PreferAntialias);

    QWidget* rootWidget = _rootWidget;
    auto __getButton = [this, rootWidget, &font] (const char* propertyName, const char* text, const char* slot, QToolButton**btnOut)->int {
        QToolButton* btn = new QToolButton(rootWidget);
        btn->setProperty(propertyName, true);
        btn->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
        btn->setCursor(QCursor(Qt::PointingHandCursor));
        btn->setText(text);
        btn->setFont(font);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btn, SIGNAL(clicked()), this, slot, Qt::QueuedConnection);
        *btnOut = btn;
        
        btn->resize(btn->sizeHint().width(), btn->sizeHint().height());
        return (btn->width() - Utils::scale_value(40)) / 2;
    };
    
    QToolButton* btn1;
    QToolButton* btn2;
    QToolButton* btn3;

    int btnTail1 = 0;
    int btnTail2 = 0;
    int btnTail3 = 0;

    btnTail1 = __getButton("VoipSysPanelControls_hangup", QT_TRANSLATE_NOOP("voip_pages","End call").toUtf8(), SLOT(_onDecline()), &btn1);
    btnTail2 = __getButton("VoipSysPanelControls_answer", QT_TRANSLATE_NOOP("voip_pages", "Answer").toUtf8(), SLOT(_onAudio()), &btn2);
    btnTail3 = __getButton("VoipSysPanelControls_video", QT_TRANSLATE_NOOP("voip_pages", "Video").toUtf8(), SLOT(_onVideo()), &btn3);

    layoutTarget->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    layoutTarget->addWidget(btn1);
    layoutTarget->addSpacing(VOIP_VIDEO_PANEL_BTN_OFFSET - btnTail1 - btnTail2);
    layoutTarget->addWidget(btn2);
    layoutTarget->addSpacing(VOIP_VIDEO_PANEL_BTN_OFFSET - btnTail2 - btnTail3);
    layoutTarget->addWidget(btn3);
    layoutTarget->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
}

Ui::VoipSysPanelControl::~VoipSysPanelControl() {
}

void Ui::VoipSysPanelControl::_onDecline() {
    emit onDecline();
}

void Ui::VoipSysPanelControl::_onAudio() {
    emit onAudio();
}

void Ui::VoipSysPanelControl::_onVideo() {
    emit onVideo();
}

void Ui::VoipSysPanelControl::changeEvent(QEvent* e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow() || _rootWidget->isActiveWindow()) {
            if (_parent) {
                _parent->blockSignals(true);
                _parent->raise();
                raise();
                _parent->blockSignals(false);
            }
        }
    }
}

Ui::VoipSysPanelHeader::VoipSysPanelHeader(QWidget* parent)
: MoveablePanel(parent)
, _nameAndStatusContainer(NULL)
, _rootWidget(NULL)
, _avatarContainer(NULL) {
    setProperty("VoipSysPanelHeader", true);
    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);  

    QVBoxLayout* root_layout = new QVBoxLayout();
    root_layout->setContentsMargins(0, 0, 0, 0);
    root_layout->setSpacing(0);
    root_layout->setAlignment(Qt::AlignVCenter);
    setLayout(root_layout);

    _rootWidget = new QWidget(this);
    _rootWidget->setContentsMargins(0, 0, 0, 0);
    _rootWidget->setProperty("VoipSysPanelHeader", true);
    layout()->addWidget(_rootWidget);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignVCenter);
    _rootWidget->setLayout(layout);

    _avatarContainer = new AvatarContainerWidget(_rootWidget, Utils::scale_value(66), Utils::scale_value(12), Utils::scale_value(12), Utils::scale_value(5));
    _avatarContainer->setOverlap(0.2f);
    _rootWidget->layout()->addWidget(_avatarContainer);

    _nameAndStatusContainer = new NameAndStatusWidget(_rootWidget, Utils::scale_value(23), Utils::scale_value(15));
    _nameAndStatusContainer->setNameProperty("VoipPanelHeaderText_Name", true);
    _nameAndStatusContainer->setStatusProperty("VoipPanelHeaderText_Status", true);
    _nameAndStatusContainer->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    _rootWidget->layout()->addWidget(_nameAndStatusContainer);

    layout->addSpacing(Utils::scale_value(12));
}

Ui::VoipSysPanelHeader::~VoipSysPanelHeader() {
    
}

void Ui::VoipSysPanelHeader::enterEvent(QEvent* e) {
	QWidget::enterEvent(e);
	emit onMouseEnter();
}

void Ui::VoipSysPanelHeader::leaveEvent(QEvent* e) {
	QWidget::leaveEvent(e);
	emit onMouseLeave();
}

void Ui::VoipSysPanelHeader::setAvatars(const std::vector<std::string> avatarList) {
    _avatarContainer->dropExcess(avatarList);
}

void Ui::VoipSysPanelHeader::setTitle(const char* s) {
    if (_nameAndStatusContainer) {
        _nameAndStatusContainer->setName(s);
    }
}

void Ui::VoipSysPanelHeader::setStatus(const char* s) {
    if (_nameAndStatusContainer) {
        _nameAndStatusContainer->setStatus(s);
    }
}

bool Ui::VoipSysPanelHeader::uiWidgetIsActive() const {
    if (_rootWidget) {
        return _rootWidget->isActiveWindow();
    }
    return false;
}