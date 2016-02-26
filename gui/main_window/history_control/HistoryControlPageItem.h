#pragma once

namespace Ui
{

	typedef std::unique_ptr<class HistoryControlPageItem> HistoryControlPageItemUptr;

	class HistoryControlPageItem : public QWidget
	{
        Q_OBJECT

    Q_SIGNALS:
        void heightChangedEvent(QSize, QSize);

    // template methods
	public:
        virtual QString formatRecentsText() const = 0;

    public:
		HistoryControlPageItem(QWidget *parent);

        virtual void clearSelection();

        bool hasAvatar() const;

        bool hasTopMargin() const;

        bool isSelected() const;

        virtual void setHasAvatar(const bool value);

        virtual void select();

        virtual void setTopMargin(const bool value);

    protected:
        virtual void resizeEvent(QResizeEvent*) override;

        virtual void showEvent(QShowEvent*) override;

    private:
        bool HasTopMargin_;

        bool HasAvatar_;

        QSize LastSize_;

        bool Selected_;

	};

}