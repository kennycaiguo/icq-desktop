#include "stdafx.h"
#include "ContextMenu.h"
#include "../utils/utils.h"

namespace Ui
{
    int MenuStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
    {
        int s = QProxyStyle::pixelMetric(metric, option, widget);
        if (metric == QStyle::PM_SmallIconSize) {
            s = Utils::scale_value(20);
        }
        return s;
    }

    ContextMenu::ContextMenu(QWidget* parent)
        : QMenu(parent)
        , InvertRight_(false)
        , Indent_(0)
    {
        setWindowFlags(windowFlags() | Qt::NoDropShadowWindowHint);
        setStyle(new MenuStyle());
        setStyleSheet(QString("QMenu {\
                       background-color: #f2f2f2;\
                       border: 1px solid #d4d4d4;\
                       }\
                       QMenu::item {\
                       padding-left: %1px;\
                       background-color: transparent;\
                       color: #282828;\
                       padding-top: %4px;\
                       padding-bottom: %4px;\
                       padding-right: %2px;\
                       }\
                       QMenu::item:selected\
                       {\
                       color: #282828;\
                       padding-left: %1px;\
                       background-color: #dcdcdc;\
                       padding-top: %4px;\
                       padding-bottom: %4px;\
                       padding-right: %2px;\
                       }\
                       QMenu::icon\
                       {\
                       padding-left:%3px;\
                       }").arg(Utils::scale_value(40)).arg(Utils::scale_value(12)).arg(Utils::scale_value(22)).arg(Utils::scale_value(8)));

        QFont font = Utils::appFont(Utils::FontsFamily::SEGOE_UI, Utils::scale_value(15));
        setFont(font);
    }

    QAction* ContextMenu::addActionWithIcon(const QIcon& icon, const QString& name, const QVariant& data)
    {
        QAction* action = addAction(icon, name);
        action->setData(data);
        return action;
    }

    QAction* ContextMenu::addActionWithIcon(const QIcon& icon, const QString& name, const QObject *receiver, const char* member)
    {
        return addAction(icon, name, receiver, member);
    }

    void ContextMenu::invertRight(bool invert)
    {
        InvertRight_ = invert;
    }

    void ContextMenu::setIndent(int indent)
    {
        Indent_ = indent;
    }

	void ContextMenu::popup(const QPoint &pos, QAction *at)
	{
		Pos_ = pos;
		QMenu::popup(pos, at);
	}

    void ContextMenu::showEvent(QShowEvent *)
    {
		if (InvertRight_ || Indent_ != 0)
		{
			QPoint p;
			if (pos().x() != Pos_.x())
				p = Pos_;
			else
				p = pos();

			if (InvertRight_)
				p.setX(p.x() - width() - Indent_);
			else
				p.setX(p.x() + Indent_);
			move(p);
		}
    }
}
