#include "stdafx.h"
#include "GeneralDialog.h"
#include "../utils/utils.h"
#include "../gui_settings.h"
#include "TextEmojiWidget.h"
#include "SemitransparentWindow.h"

namespace Ui
{
	GeneralDialog::GeneralDialog(bool _isShowLabel, bool _isShowButton, QString _text_label, QString _button_text, QWidget* _main_widget, QWidget* _parent,
        int _button_margin_dip)
		: QDialog(_parent)
		, main_widget_(_main_widget)
        , isShowButton_(_isShowButton)
        , isShowLabel_(_isShowLabel)
        , button_margin_(_button_margin_dip)
        , button_text_(_button_text)
	{
		const QString close_button_style =
            "QPushButton { image: url(:/resources/main_window/contr_close_100.png); width: 24dip; height: 24dip; background-color: transparent; margin: 0; padding-left: 7dip; padding-top: 2dip; padding-right: 7dip; padding-bottom: 2dip; border: none; } "
			"QPushButton:hover { image: url(:/resources/main_window/contr_close_100_hover.png); background-color: #e81123; } "
			"QPushButton:pressed { image: url(:/resources/main_window/contr_close_100_active.png); background-color: #d00516; }";

        const QString next_button_style =
            "QPushButton[default=\"false\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; color: #282828; font-size: 18dip; background-color: #dcdcdc; border: none; padding: 0; margin: 0; padding-left: 32dip; padding-right: 32dip; max-height: 40dip; min-height: 40dip; } QPushButton[default=\"false\"]:hover { background-color: #e5e5e5; } QPushButton[default=\"false\"]:pressed { background-color: #d5d5d5; }"
            "QPushButton[default=\"true\"] { font-family: \"%FONT_FAMILY%\"; font-weight: %FONT_WEIGHT%; color: #579e1c; font-size: 18dip; background-color: #66cae6b3; border-style: solid; border-width: 2dip; border-color: #579e1c; margin: 0; padding: 0; padding-left: 32dip; padding-right: 32dip; max-height: 36dip; min-height: 36dip;} QPushButton[default=\"true\"]:hover { color: #ffffff; background-color: #579e1c; }";

        if (platform::is_apple())
            scene_ = this; // TODO: will be removed later
        else
            scene_ = this;
        
        auto global_layout = new QVBoxLayout(scene_);
        global_layout->setMargin(0);
		global_layout->setSpacing(0);
        global_layout->setAlignment(Qt::AlignTop);
        
        if (isShowLabel_)
        {
            {
                auto host = new QWidget(scene_);
                auto host_layout = new QHBoxLayout(host);
                host_layout->setContentsMargins(0, 0, 0, 0);
                host_layout->setSpacing(0);
                host_layout->setAlignment(Qt::AlignRight);
                host->setStyleSheet("background-color: white;");
                {
                    auto close_button = new QPushButton(host);
                    close_button->setFlat(true);
                    close_button->setCursor(Qt::PointingHandCursor);
                    Utils::ApplyStyle(close_button, close_button_style);
                    QObject::connect(close_button, SIGNAL(clicked()), this, SLOT(reject()), Qt::QueuedConnection);
                    host_layout->addWidget(close_button);
                }
                global_layout->addWidget(host);
            }
            {
                auto host = new QWidget(scene_);
                auto host_layout = new QHBoxLayout(host);
                host_layout->setContentsMargins(Utils::scale_value(24), 0, Utils::scale_value(24), 0);
                host_layout->setSpacing(0);
                host_layout->setAlignment(Qt::AlignLeft);
                host->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                host->setStyleSheet("background-color: white;");
                {
                    auto label = new TextEmojiWidget(scene_, Utils::FontsFamily::SEGOE_UI, Utils::scale_value(24), QColor("#282828"), Utils::scale_value(22));
                    label->setSizePolicy(QSizePolicy::Policy::Preferred, label->sizePolicy().verticalPolicy());
                    label->setText(_text_label);
                    host_layout->addWidget(label);
                }
                global_layout->addWidget(host);
            }
        }

        global_layout->addWidget(main_widget_);
                
        if (isShowButton_)
        {
            auto bottom_widget = new QWidget(scene_);
            auto bottom_layout = new QHBoxLayout(bottom_widget);
            bottom_layout->setContentsMargins(0, Utils::scale_value(button_margin_), 0, Utils::scale_value(button_margin_));
            bottom_layout->setSpacing(0);
            bottom_layout->setAlignment(Qt::AlignCenter);
            bottom_widget->setStyleSheet("background-color: white;");
            bottom_widget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
            {
                next_button_ = new QPushButton(bottom_widget);
                Utils::ApplyStyle(next_button_, next_button_style);
                setButtonActive(true);
                next_button_->setFlat(true);
                next_button_->setCursor(QCursor(Qt::PointingHandCursor));
                next_button_->setText(button_text_);
                next_button_->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
                QObject::connect(next_button_, SIGNAL(clicked()), this, SLOT(accept()), Qt::QueuedConnection);
                bottom_layout->addWidget(next_button_);
            }
            global_layout->addWidget(bottom_widget);
            
            Testing::setAccessibleName(next_button_, "next_button_");
        }
        
        scene_->setStyleSheet("background-color: white; border: none;");
        scene_->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowSystemMenuHint);
        scene_->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        scene_->setFixedWidth(Utils::scale_value(360));

        main_widget_->setStyleSheet("background-color: white;");

        Utils::addShadowToWindow(scene_, true);
        semi_window_ = new SemitransparentWindow(Ui::get_gui_settings(), scene_->parentWidget());
        semi_window_->setShow(false);
        
        scene_rect_ = scene_->rect();
	}

	GeneralDialog::~GeneralDialog()
	{
        delete semi_window_;
	}

    void GeneralDialog::on_resize_child(int _delta_w, int _delta_h)
    {
        const auto shadow_width = Ui::get_gui_settings()->get_shadow_width();
        const auto old_height = (height_ == -1 ? (scene_->rect().height() - 2 * shadow_width) : height_);
        const auto old_width = (width_ == -1 ? (scene_->rect().width() - 2 * shadow_width) : width_);
        updateParamsRoutine(old_width + _delta_w, old_height + _delta_h, x_, y_, is_semi_window_, is_margined_);
    }

	bool GeneralDialog::showWithFixedSizes(int _width, int _height, int _x, int _y)
    {
        updateParamsRoutine(_width, _height, _x, _y, false, true);
        const auto result = (exec() == QDialog::Accepted);
        if (platform::is_apple())
            updateParamsRoutine(_width, _height, _x, _y, false, false);
		return result;
	}

    void GeneralDialog::setButtonActive(bool _active)
    {
        if (isShowButton_)
            Utils::ApplyPropertyParameter(next_button_, "default", _active);
    }

    void GeneralDialog::updateParams(int _width, int _height, int _x, int _y, bool _is_semi_window)
    {
        updateParamsRoutine(_width, _height, _x, _y, _is_semi_window);
    }

    void GeneralDialog::updateParamsRoutine(int _width, int _height, int _x, int _y, bool _is_semi_window, bool margined/* = false*/)
    {
        const auto shadow_width = Ui::get_gui_settings()->get_shadow_width();
        const auto& rect = Ui::get_gui_settings()->get_value(settings_main_window_rect_real, QRect());
        
        if (platform::is_apple() && margined)
        {
            if (_width == -1)
                _width = scene_->sizeHint().width();
            if (_height == -1)
                _height = scene_->sizeHint().height();
            _width += (2 * shadow_width);
            _height += (2 * shadow_width);
            scene_->setFixedSize(rect.size());
            scene_->move(rect.topLeft());
            const auto mx = abs(_x - rect.topLeft().x());
            const auto my = abs(_y - rect.topLeft().y() + Utils::scale_value(8));
            const auto mw = abs(rect.width() - _width - mx);
            const auto mh = abs(rect.height() - _height - my);
            scene_->layout()->setContentsMargins(mx, my, mw, mh);
            scene_rect_ = QRect(_x, _y + Utils::scale_value(8), _width, _height);
        }
        else
        {
            scene_->layout()->setContentsMargins(0, 0, 0, 0);
            if (_width != -1)
                scene_->setFixedWidth(_width + 2 * shadow_width);
            if (_height != -1)
                scene_->setFixedHeight(_height + 2 * shadow_width);
            scene_->move(_x, _y + (platform::is_apple() ? Utils::scale_value(8) : 0));
            scene_rect_ = scene_->rect();
        }

        semi_window_->setShow(_is_semi_window);
        
        width_ = _width;
        height_ = _height;
        x_ = _x;
        y_ =  _y;
        is_semi_window_ = _is_semi_window;
        is_margined_ = margined;
    }

    void GeneralDialog::hideEvent(QHideEvent *e)
    {
        semi_window_->setShow(false);
        QDialog::hideEvent(e);
    }

    void GeneralDialog::mousePressEvent(QMouseEvent* e)
    {
        QDialog::mousePressEvent(e);
        const auto p = mapToGlobal(e->pos());
        if (platform::is_apple() && !scene_rect_.contains(p))
            close();
    }

}