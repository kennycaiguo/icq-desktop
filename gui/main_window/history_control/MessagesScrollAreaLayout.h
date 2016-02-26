#pragma once

namespace Ui
{

    class MessagesScrollbar;
    class MessagesScrollArea;

    class MessagesScrollAreaLayout : public QLayout
    {
        enum class SlideOp;

        friend QTextStream& operator <<(QTextStream &lhs, const SlideOp slideOp);

    public:
        typedef std::function<bool(Ui::MessageItem*, const bool)> MessageItemVisitor;

        typedef std::function<bool(QWidget*, const bool)> WidgetVisitor;

        MessagesScrollAreaLayout(
            MessagesScrollArea *scrollArea,
            MessagesScrollbar *messagesScrollbar,
            QWidget *typingWidget
        );

        virtual void setGeometry(const QRect &r) override;

        virtual void addItem(QLayoutItem *item) override;

        virtual QLayoutItem *itemAt(int index) const override;

        virtual QLayoutItem *takeAt(int index) override;

        virtual int count() const override;

        virtual QSize sizeHint() const override;

        virtual void invalidate() override;

        bool containsWidget(QWidget *widget) const;

        QWidget* getItemByPos(const int32_t pos) const;

        int32_t getItemsHeight() const;

        int32_t getViewportAbsY() const;

        QRect getViewportScrollBounds() const;

        int32_t getViewportHeight() const;

        void insertWidget(const int32_t pos, QWidget *widget);

        void removeWidget(QWidget *widget);

        void setViewportByOffset(const int32_t bottomOffset);

        int32_t shiftViewportAbsY(const int32_t delta);

        void enumerateMessagesItems(const MessageItemVisitor visitor, const bool reversed);

        void enumerateWidgets(const WidgetVisitor visitor, const bool reversed);

        bool isViewportAtBottom() const;

    private:
        struct ItemInfo
        {
            ItemInfo(QWidget *widget);

            QWidget *Widget_;

            QRect AbsGeometry_;
        };

        typedef std::deque<ItemInfo> ItemsInfo;

        typedef ItemsInfo::iterator ItemsInfoIter;

        std::set<QWidget*> Widgets_;

        ItemsInfo LayoutItems_;

        MessagesScrollbar *Scrollbar_;

        MessagesScrollArea *ScrollArea_;

        QWidget *TypingWidget_;

        QRect LayoutRect_;

        QSize ViewportSize_;

        int32_t ViewportAbsY_;

        bool IsDirty_;

        bool UpdatesLocked_;

        QRect absolute2Viewport(const QRect &absolute) const;

        void applyItemsGeometry();

        void applyTypingWidgetGeometry();

        QRect calculateInsertionRect(const ItemsInfoIter &itemInfoIter, Out SlideOp &slideOp);

        void debugValidateGeometry();

        int32_t evalViewportAbsMiddleY() const;

        QRect getItemsAbsBoundingRect() const;

        int32_t getTypingWidgetHeight() const;

        ItemsInfoIter insertItem(QWidget *widget, const int32_t position);

        bool setViewportAbsY(const int32_t absY);

        bool slideItemsApart(const ItemsInfoIter &changedItemIter, const int slideY, const SlideOp slideOp);

        void updateItemsWidth();

        void updateItemsGeometry();

        void moveViewportToBottom();
    };

}