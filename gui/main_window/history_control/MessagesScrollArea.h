#pragma once

namespace Ui
{
    class MessagesScrollbar;
    class MessageItem;
    class HistoryControlPageItem;
    class MessagesScrollAreaLayout;

    class MessagesScrollArea : public QWidget
    {
        Q_OBJECT

    Q_SIGNALS:

        void fetchRequestedEvent();

        void needCleanup();

        void scrollMovedToBottom();

    public:
        typedef std::function<bool(Ui::MessageItem*, const bool)> MessageItemVisitor;

        typedef std::function<bool(QWidget*, const bool)> WidgetVisitor;

        MessagesScrollArea(QWidget *parent, QWidget *typingWidget);

        void cancelSelection();

        void enumerateMessagesItems(const MessageItemVisitor visitor, const bool reversed) const;

        void enumerateWidgets(const WidgetVisitor visitor, const bool reversed) const;

        QWidget* getItemByPos(const int32_t pos) const;

        QString getSelectedText() const;

        void insertWidget(const int32_t pos, QWidget *widget);

        bool isSelecting() const;

        bool isViewportFull() const;

        void removeWidget(QWidget *widget);

        bool touchScrollInProgress() const;

        void scrollToBottom();

        void updateScrollbar();

        bool isScrollAtBottom() const;

    protected:
        virtual void mouseMoveEvent(QMouseEvent *e) override;

        virtual void mousePressEvent(QMouseEvent *e) override;

        virtual void mouseReleaseEvent(QMouseEvent *e) override;

        virtual void wheelEvent(QWheelEvent *e) override;

        virtual bool event(QEvent *e) override;

    private Q_SLOTS:
        void onAnimationTimer();

        void onMessageHeightChanged(QSize, QSize);

        void onSliderMoved(int value);

        void onSliderPressed();

        void onSliderValue(int value);

        void onIdleUserActivityTimeout();

    private:
        enum class ScrollingMode;

        bool IsSelecting_;

        bool TouchScrollInProgress_;

        int64_t LastAnimationMoment_;

        QPoint LastMouseGlobalPos_;

        ScrollingMode Mode_;

        MessagesScrollbar *Scrollbar_;

        MessagesScrollAreaLayout *Layout_;

        QTimer ScrollAnimationTimer_;

        double ScrollDistance_;

        bool Resizing_;

        QPointF PrevTouchPoint_;

        bool IsUserActive_;

        QTimer UserActivityTimer_;

        void applySelection();

        void clearSelection();

        double evaluateScrollingSpeed() const;

        double evaluateScrollingStep(const int64_t now) const;

        bool isScrolling() const;

        void startScrollAnimation(const ScrollingMode mode);

        void stopScrollAnimation();

        void unloadWidgets();

        void resetUserActivityTimer();
    };

}