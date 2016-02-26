#include "stdafx.h"
#include "../utils/utils.h"
#include "BackgroundWidget.h"

namespace Ui
{
    BackgroundWidget::BackgroundWidget(QWidget* _parent, const QString& _imageName)
    : QStackedWidget(_parent)
    {
        if (_imageName.length() > 0)
        {
            pixmapToDraw_ = QPixmap(Utils::parse_image_name(_imageName));
        }
    };

    void BackgroundWidget::paintEvent(QPaintEvent *_e)
    {
        QWidget::paintEvent(_e);
        
        if (!pixmapToDraw_.isNull())
        {
            QPainter painter(this);
            Utils::check_pixel_ratio(pixmapToDraw_);
            double ratio = Utils::scale_bitmap(1);
            
            for (int x = 0; x < this->rect().width(); x += pixmapToDraw_.width()/ratio)
            {
                for (int y = 0; y < this->rect().height(); y += pixmapToDraw_.height()/ratio)
                {
                    painter.drawPixmap(x, y, pixmapToDraw_);
                }
            }
        }
    }
	
    void BackgroundWidget::setImage(const QString& _imageName)
    {
        if (_imageName.length() > 0)
        {
            pixmapToDraw_ = QPixmap(Utils::parse_image_name(_imageName));
        }
        else
        {
            pixmapToDraw_ = QPixmap();
        }
        update();
    }
	
}