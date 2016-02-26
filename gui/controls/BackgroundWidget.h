#pragma once

namespace Ui {

	class BackgroundWidget : public QStackedWidget
	{
		QPixmap		pixmapToDraw_;

	protected:
		virtual void paintEvent(QPaintEvent*) override;

	public:

		BackgroundWidget(QWidget* _parent, const QString& _imageName);

		void setImage(const QString& _imageName);
	};

}