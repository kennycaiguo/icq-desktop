#include "stdafx.h"
#include "SemitransparentWindow.h"
#include "../gui_settings.h"
#include "../main_window/contact_list/Common.h"

namespace Ui
{
	SemitransparentWindow::SemitransparentWindow(qt_gui_settings* _qui_settings, QWidget* _parent)
		: QWidget(_parent)
	{
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowSystemMenuHint);
		setPalette(QPalette(Qt::black));
        setShow(true);
     
        const auto &rect = _qui_settings->get_value(settings_main_window_rect_real, QRect());
        auto width = ::ContactList::ItemLength(true, 1, ::ContactList::dip(0)).px();
        auto height = ::ContactList::ItemLength(false, 1, ::ContactList::dip(0)).px();
		resize(width, height);
        move(rect.x(), rect.y());
		show();
	}

	SemitransparentWindow::~SemitransparentWindow()
	{
	}

    void SemitransparentWindow::setShow(bool _is_show)
    {
        setWindowOpacity(_is_show ? 0.4 : 0);
    }
}