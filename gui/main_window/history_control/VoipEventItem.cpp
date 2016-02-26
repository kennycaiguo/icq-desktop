#include "stdafx.h"

#include "../../core_dispatcher.h"

#include "../../cache/avatars/AvatarStorage.h"

#include "../../utils/PainterPath.h"
#include "../../utils/utils.h"

#include "../../themes/ThemePixmap.h"

#include "VoipEventInfo.h"

#include "VoipEventItem.h"

namespace Ui
{

    namespace
    {
        int32_t getAvatarRightMargin();

        int32_t getAvatarSize();

        QBrush getBodyBrush(const bool isOutgoing);

        int32_t getBodyHeight();

        QBrush getBodyHoveredBrush();

        int32_t getIconLeftPadding();

        int32_t getIconRightPadding();

        int32_t getIconTopPadding();

        int32_t getLeftPadding(const bool isOutgoing);

        int32_t getRightPadding(const bool isOutgoing);

        int32_t getTextBaselineY();

        QColor getTextColor(const bool isHovered);

        const QFont& getTextFont();

        int32_t getTopPadding(const bool hasTopMargin);
    }

    VoipEventItem::VoipEventItem(const HistoryControl::VoipEventInfoSptr& eventInfo)
        : HistoryControlPageItem(nullptr)
        , EventInfo_(eventInfo)
    {
        assert(EventInfo_);
    }

    VoipEventItem::VoipEventItem(
        QWidget *parent,
        const HistoryControl::VoipEventInfoSptr& eventInfo)
        : HistoryControlPageItem(parent)
        , EventInfo_(eventInfo)
        , IsHovered_(false)
    {
        assert(EventInfo_);

        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        if (!EventInfo_->isVisible())
        {
            assert(!"invisible voip events are not allowed in history control");
            setFixedHeight(0);
            return;
        }

        setAttribute(Qt::WA_TranslucentBackground);

        if (EventInfo_->isClickable())
        {
            setMouseTracking(true);
        }

        Icon_ = eventInfo->loadIcon(false);
        HoverIcon_ = eventInfo->loadIcon(true);
    }

    QString VoipEventItem::formatRecentsText() const
    {
        return EventInfo_->formatEventText();
    }

    void VoipEventItem::setTopMargin(const bool value)
    {
        if (!EventInfo_->isVisible())
        {
            return;
        }

        setFixedHeight(
            getBodyHeight() + getTopPadding(value)
        );

        HistoryControlPageItem::setTopMargin(value);
    }

    void VoipEventItem::setHasAvatar(const bool value)
    {
        if (!isOutgoing() && value)
        {
            auto isDefault = false;
            Avatar_ = Logic::GetAvatarStorage()->GetRounded(
                EventInfo_->getContactAimid(),
                EventInfo_->getContactFriendly(),
                Utils::scale_value(32),
                QString(),
                true,
                Out isDefault
            );

            assert(Avatar_);
        }
        else
        {
            Avatar_.reset();
        }

        HistoryControlPageItem::setHasAvatar(value);
    }

    void VoipEventItem::mouseMoveEvent(QMouseEvent *event)
    {
        if (!EventInfo_->isClickable())
        {
            return;
        }

        const auto pos = event->pos();

        const auto isHovered = (
            (pos.y() > getTopPadding(hasTopMargin())) &&
            (pos.x() > BubbleRect_.left()) &&
            (pos.x() < BubbleRect_.right())
        );
        if (IsHovered_ == isHovered)
        {
            return;
        }

        if (isHovered)
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }

        IsHovered_ = isHovered;
        update();
    }

    void VoipEventItem::mousePressEvent(QMouseEvent *)
    {
        if (!IsHovered_)
        {
            return;
        }

        const auto &contactAimid = EventInfo_->getContactAimid();
        assert(!contactAimid.isEmpty());
        Ui::GetDispatcher()->getVoipController().setStartV(contactAimid.toUtf8(), false);
    }

    void VoipEventItem::leaveEvent(QEvent *)
    {
        if (!EventInfo_->isClickable())
        {
            return;
        }

        IsHovered_ = false;
        update();
    }

    void VoipEventItem::paintEvent(QPaintEvent *)
    {
        if (!BubbleRect_.isValid())
        {
            return;
        }

        QPainter p(this);

        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);

        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);

        if (Bubble_.isEmpty())
        {
            Bubble_ = Utils::renderMessageBubble(BubbleRect_, Utils::scale_value(8), isOutgoing());
            assert(!Bubble_.isEmpty());
        }

        assert(BubbleRect_.width() > 0);

        const auto bodyBrush = (
            IsHovered_ ?
                getBodyHoveredBrush() :
                getBodyBrush(isOutgoing())
        );

        p.fillPath(Bubble_, bodyBrush);

        const auto baseY = BubbleRect_.top();

        auto cursorX = getLeftPadding(isOutgoing());

        if (!isOutgoing())
        {
            if (Avatar_)
            {
                p.drawPixmap(cursorX, baseY, *Avatar_);
            }

            cursorX += getAvatarSize();
            cursorX += getAvatarRightMargin();
        }

        auto &icon = (IsHovered_ ? HoverIcon_ : Icon_);
        if (icon)
        {
            cursorX += getIconLeftPadding();

            icon->Draw(p, cursorX, baseY + getIconTopPadding());
            cursorX += icon->GetWidth();

            cursorX += getIconRightPadding();
        }
        else
        {
            cursorX += getIconLeftPadding();
        }

        const auto eventText = (
            IsHovered_ ?
                QT_TRANSLATE_NOOP("chat_event", "Call back") :
                EventInfo_->formatEventText()
        );

        p.setPen(getTextColor(IsHovered_));
        p.setFont(getTextFont());
        p.drawText(
            cursorX,
            baseY + getTextBaselineY(),
            eventText
        );
    }

    void VoipEventItem::resizeEvent(QResizeEvent *event)
    {
        QRect newBubbleRect(QPoint(0, 0), event->size());

        QMargins margins(
            getLeftPadding(isOutgoing()),
            getTopPadding(hasTopMargin()),
            getRightPadding(isOutgoing()),
            0
        );

        if (!isOutgoing())
        {
            margins.setLeft(
                margins.left() + getAvatarSize() + getAvatarRightMargin()
            );
        }

        newBubbleRect = newBubbleRect.marginsRemoved(margins);

        if (BubbleRect_ != newBubbleRect)
        {
            BubbleRect_ = (newBubbleRect.isValid() ? newBubbleRect : QRect());
            Bubble_ = QPainterPath();
        }

        HistoryControlPageItem::resizeEvent(event);
    }

    bool VoipEventItem::isOutgoing() const
    {
        return !EventInfo_->isIncomingCall();
    }

    namespace
    {
        int32_t getAvatarRightMargin()
        {
            return Utils::scale_value(6);
        }

        int32_t getAvatarSize()
        {
            return Utils::scale_value(32);
        }

        int32_t getIconLeftPadding()
        {
            return Utils::scale_value(16);
        }

        int32_t getIconRightPadding()
        {
            return Utils::scale_value(12);
        }

        int32_t getIconTopPadding()
        {
            return Utils::scale_value(9);
        }

        QBrush getBodyBrush(const bool isOutgoing)
        {
            QLinearGradient grad(0, 0, 1, 0);

            grad.setCoordinateMode(QGradient::ObjectBoundingMode);

            const auto color0 = (
                isOutgoing ?
                    QColor(0xd8, 0xd4, 0xce, (int32_t)(0.9 * 255)) :
                    QColor(0xff, 0xff, 0xff, (int32_t)(1.0 * 255))
            );
            grad.setColorAt(0, color0);

            const auto color1 = (
                isOutgoing ?
                    QColor(0xd5, 0xd2, 0xce, (int32_t)(0.72 * 255)) :
                    QColor(0xff, 0xff, 0xff, (int32_t)(0.72 * 255))
            );
            grad.setColorAt(1, color1);

            QBrush result(grad);
            result.setColor(QColor(0, 0, 0, 0));

            assert(!result.isOpaque());
            return result;
        }

        int32_t getBodyHeight()
        {
            return Utils::scale_value(32);
        }

        QBrush getBodyHoveredBrush()
        {
            QLinearGradient grad(0, 0, 1, 0);

            grad.setCoordinateMode(QGradient::ObjectBoundingMode);

            const QColor color0(0x57, 0x9e, 0x1c, (int32_t)(0.9 * 255));
            grad.setColorAt(0, color0);

            const QColor color1(0x57, 0x9e, 0x1c, (int32_t)(0.72 * 255));
            grad.setColorAt(1, color1);

            QBrush result(grad);
            result.setColor(QColor(0, 0, 0, 0));

            assert(!result.isOpaque());
            return result;
        }

        int32_t getLeftPadding(const bool isOutgoing)
        {
            return Utils::scale_value(
                isOutgoing ? 118 : 24
            );
        }

        int32_t getRightPadding(const bool isOutgoing)
        {
            return Utils::scale_value(
                isOutgoing ? 24 : 80
            );
        }

        int32_t getTextBaselineY()
        {
            return Utils::scale_value(21);
        }

        QColor getTextColor(const bool isHovered)
        {
            return (isHovered ? 0xffffff : 0x282828);
        }

        const QFont& getTextFont()
        {
            static QFont font(
                Utils::appFont(Utils::FontsFamily::SEGOE_UI, Utils::scale_value(15))
            );

            return font;
        }

        int32_t getTopPadding(const bool hasTopMargin)
        {
            return Utils::scale_value(
                hasTopMargin ? 12 : 2
            );
        }
    }

}