#pragma once

namespace Ui {
    
class CustomButton : public QPushButton
{
    virtual void paintEvent(QPaintEvent*) override;

    QPixmap		pixmapToDraw_;
    QPixmap		pixmapDefault_;
    QPixmap		pixmapHover_;
    QPixmap		pixmapActive_;
	QColor		fillColor_;

	int			x_;
	int			y_;
	int			xForActive_;
	int			yForActive_;
	bool		activeState_;
    int         align_;

    void leaveEvent(QEvent * _e) override;
    void enterEvent(QEvent * _e) override;
    
public:
    
    CustomButton(QWidget* _parent, const QString& _imageName);

    void setAlign(int flags);
    void setOffsets(int _x, int _y);
	void setOffsetsForActive(int _x, int _y);
    void setImage(const QString& _imageName);
    void setHoverImage(const QString& _imageName);
    void setActiveImage(const QString& _imageName);
    void setActive(bool _isActive);
	void setFillColor(QColor);
};
    
}

