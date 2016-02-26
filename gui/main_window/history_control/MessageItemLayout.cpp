#include "stdafx.h"

#include "../../utils/profiling/auto_stop_watch.h"

#include "MessageItem.h"

#include "MessageItemLayout.h"

namespace Ui
{
    MessageItemLayout::MessageItemLayout(MessageItem *parent)
        : QLayout(parent)
        , Width_(-1)
        , IsDirty_(false)
    {
        assert(parent);
    }

    void MessageItemLayout::setGeometry(const QRect &r)
    {
        QLayout::setGeometry(r);

        const auto widthChanged = (Width_ != r.width());
        if (!widthChanged && !IsDirty_)
        {
            return;
        }

        Width_ = r.width();

        IsDirty_ = false;

        auto item = qobject_cast<MessageItem*>(parent());
        item->manualUpdateGeometry(Width_);
    }

    void MessageItemLayout::addItem(QLayoutItem *item)
    {
        item;
    }

    QLayoutItem* MessageItemLayout::itemAt(int index) const
    {
        index;
        return nullptr;
    }

    QLayoutItem* MessageItemLayout::takeAt(int index)
    {
        index;
        return nullptr;
    }

    int MessageItemLayout::count() const
    {
        return 0;
    }

    QSize MessageItemLayout::sizeHint() const
    {
        auto item = qobject_cast<MessageItem*>(parent());
        return item->sizeHint();
    }

    void MessageItemLayout::invalidate()
    {
        QLayout::invalidate();
    }

    void MessageItemLayout::setDirty()
    {
        IsDirty_ = true;
    }
}