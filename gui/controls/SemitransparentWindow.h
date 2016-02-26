#pragma once

namespace Ui
{
	class qt_gui_settings;

	class SemitransparentWindow : public QWidget
	{
		Q_OBJECT
        
	public:
		SemitransparentWindow(qt_gui_settings* _qui_settings, QWidget* _parent);
		~SemitransparentWindow();
        void setShow(bool _is_show);
	};
}