#include "stdafx.h"

#include "../../utils/log/log.h"
#include "../../utils/PainterPath.h"
#include "../../utils/profiling/auto_stop_watch.h"
#include "../../utils/Text.h"
#include "../../utils/Text2DocConverter.h"
#include "../../utils/utils.h"

#include "../../themes/ThemePixmap.h"
#include "../../themes/ResourceIds.h"

#include "../../controls/TextEditEx.h"

#include "ResizePixmapTask.h"

#include "PreviewContentWidget.h"

namespace HistoryControl
{
    namespace
    {
        QBrush getBodyBrush(const bool isOutgoing, const bool isSelected);

        int32_t getBubbleHorPadding();

        int32_t getBubbleVertPadding();

        const QSizeF& getMaxPreviewSize();

        const QSizeF& getMinPreviewSize();

        const QFont& getPreviewTextFont();

        int32_t getTextBottomMargin();

        int32_t getTextBubbleBorderRadius();
    }

    PreviewContentWidget::PreviewContentWidget(QWidget *parent, const bool isOutgoing, const QString &text, const bool previewsEnabled)
        : MessageContentWidget(parent, isOutgoing)
        , Text_(text)
        , TextControl_(nullptr)
        , IsTextVisible_(false)
        , PreviewsEnabled_(previewsEnabled)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        setMinimumSize(getMinPreviewSize().toSize());

        updateGeometry();
    }

    QPoint PreviewContentWidget::deliveryStatusOffsetHint(const int32_t statusLineWidth) const
    {
        if (TextControl_ && TextControl_->isVisible())
        {
            const auto textSize = getTextBubbleSize();
            return QPoint(
                textSize.width() - statusLineWidth,
                textSize.height()
            );
        }

        if (PreviewGenuineSize_.isValid())
        {
            return QPoint(
                getPreviewScaledRect().width(),
                0
            );
        }

        return MessageContentWidget::deliveryStatusOffsetHint(statusLineWidth);
    }

    void PreviewContentWidget::initialize()
    {
        MessageContentWidget::initialize();
    }

    QSize PreviewContentWidget::sizeHint() const
    {
        if (PreviewGenuineSize_.isEmpty())
        {
            return getMinPreviewSize().toSize();
        }

        const auto &previewScaledSize = getPreviewScaledSizeF();

        const auto bubbleSize = getTextBubbleSize();

        const auto width = previewScaledSize.width();
        const auto height = (
            bubbleSize.height() +
            getTextBottomMargin() +
            previewScaledSize.height()
        );

        QSize widgetSize(
            std::max(width, getMinPreviewSize().width()),
            std::max(height, getMinPreviewSize().height())
        );

        return widgetSize;
    }

    const QString& PreviewContentWidget::getText() const
    {
        return Text_;
    }

    void PreviewContentWidget::renderPreview(QPainter &p)
    {
        if (Preview_.isNull())
        {
            if (PreviewGenuineSize_.isValid())
            {
                const auto previewSize = getPreviewScaledRect().size();
                if (previewSize != LastSize_)
                {
                    prepareTextGeometry();
                    setFixedSize(previewSize);
                }

                LastSize_ = previewSize;

                renderPreloader(p);
            }
            else
            {
                if (TextControl_)
                {
                    prepareTextGeometry();

                    const QSize newSize(
                        width(),
                        getTextBubbleSize().height()
                    );

                    if (newSize != LastSize_)
                    {
                        setFixedSize(newSize);

                        LastSize_ = newSize;
                    }
                }
            }

            return;
        }

        auto imageRect = getPreviewScaledRect();

        imageRect = QRect(
            imageRect.left(),
            imageRect.top(),
            std::max(imageRect.width(), 2),
            std::max(imageRect.height(), 2)
        );

        auto widgetHeight = imageRect.height();

        if (TextControl_)
        {
            widgetHeight += getTextBubbleSize().height();
            widgetHeight += getTextBottomMargin();
        }

        const QSize widgetSize(
            width(),
            widgetHeight
        );

        if (LastSize_ != widgetSize)
        {
            prepareTextGeometry();
            MessageContentWidget::setFixedSize(widgetSize);
        }

        LastSize_ = widgetSize;

        p.save();

        p.drawPixmap(imageRect, Preview_);

        if (isSelected())
        {
            const QBrush brush(Utils::getSelectionColor());
            p.fillRect(imageRect, brush);
        }

        p.restore();
    }

    void PreviewContentWidget::renderTextBubble(QPainter &p)
    {
        const auto &bubblePath = getTextBubble();
        if (bubblePath.isEmpty())
        {
            return;
        }

        p.save();

        p.fillPath(
            bubblePath,
            getBodyBrush(isOutgoing(), isSelected())
        );

        p.restore();
    }

    QPainterPath PreviewContentWidget::evaluateClippingPath() const
    {
        const auto borderRadius = Utils::scale_value(8);

        auto pathSize = getPreviewScaledSizeF().toSize();
        assert(pathSize.isValid());

        auto path = Utils::renderMessageBubble(pathSize, borderRadius, isOutgoing());

        if (TextControl_)
        {
            path.translate(
                0,
                getTextBubbleSize().height() + getTextBottomMargin()
            );
        }

        return path;
    }

    void PreviewContentWidget::render(QPainter &p)
    {
        p.save();

        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);

        applyClippingPath(p);

        renderPreview(p);

        p.restore();

        renderTextBubble(p);
    }

    void PreviewContentWidget::resizeEvent(QResizeEvent *e)
    {
        MessageContentWidget::resizeEvent(e);

        invalidateSizes();
    }

    void PreviewContentWidget::setPreview(const QPixmap &preview)
    {
        assert(!preview.isNull());

        __TRACE(
            "preview",
            "setting preview\n" <<
            toLogString() << "\n"
            "----------------------------\n"
            "    preview_size=<" << preview.size() << ">"
        );

        if (PreviewGenuineSize_.isEmpty())
        {
            setPreviewGenuineSize(preview.size());
        }

        Preview_ = preview;

        limitPreviewSize();

        invalidateSizes();

        update();
    }

    void PreviewContentWidget::setPreviewGenuineSize(const QSize &size)
    {
        assert(!size.isEmpty());
        assert(PreviewGenuineSize_.isEmpty());
        assert(Preview_.isNull());

        PreviewGenuineSize_ = size;

        updateGeometry();
    }

    void PreviewContentWidget::setTextVisible(const bool isVisible)
    {
        IsTextVisible_ = isVisible;

        if (IsTextVisible_)
        {
            if (!TextControl_)
            {
                createTextControl();
            }
        }
        else
        {
            if(TextControl_)
            {
                TextControl_->hide();
                delete TextControl_;
                TextControl_ = nullptr;
            }
        }

        TextSize_ = QSize();

        update();
    }

    void PreviewContentWidget::onPreviewSizeLimited(QPixmap preview)
    {
        assert(preview);

        Preview_ = preview;
    }

    void PreviewContentWidget::applyClippingPath(QPainter &p)
    {
        if (PreviewGenuineSize_.isEmpty())
        {
            return;
        }

        if (ClippingPath_.isEmpty())
        {
            ClippingPath_ = evaluateClippingPath();
            assert(!ClippingPath_.isEmpty());
        }

        p.setClipPath(ClippingPath_);
    }

    void PreviewContentWidget::createTextControl()
    {
        assert(IsTextVisible_);
        assert(!Text_.isEmpty());
        assert(!TextControl_);

        TextControl_ = new Ui::TextEditEx(
            this,
            Utils::FontsFamily::SEGOE_UI,
            Utils::scale_value(15),
            QColor(0x28, 0x28, 0x28),
            false,
            false
        );

        TextControl_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        TextControl_->setStyle(QApplication::style());
        TextControl_->setFrameStyle(QFrame::NoFrame);
        TextControl_->setDocument(TextControl_->document());
        TextControl_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        TextControl_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        TextControl_->setOpenLinks(true);
        TextControl_->setOpenExternalLinks(true);
        TextControl_->setWordWrapMode(QTextOption::WordWrap);
        TextControl_->setStyleSheet("background: transparent");
        TextControl_->document()->setDocumentMargin(0);

        TextControl_->verticalScrollBar()->blockSignals(true);
        Logic::Text4Edit(Text_, *TextControl_, Logic::Text2DocHtmlMode::Escape, true, true);
        TextControl_->verticalScrollBar()->blockSignals(false);

        TextControl_->show();
    }

    QSizeF PreviewContentWidget::evaluatePreviewScaledSize(const int boundWidth) const
    {
        assert(PreviewGenuineSize_.isValid());

        QSizeF fixedSize(boundWidth, 0);

        // prevent small images from being stretched up
        fixedSize.setWidth(
            std::min(fixedSize.width(), PreviewGenuineSize_.width())
        );

        const auto verticalAspectRatio = ((double)PreviewGenuineSize_.height() / (double)PreviewGenuineSize_.width());
        fixedSize.setHeight(fixedSize.width() * verticalAspectRatio);

        const auto shouldScaleDown =
            (fixedSize.width() > getMaxPreviewSize().width()) ||
            (fixedSize.height() > getMaxPreviewSize().height());
        if (shouldScaleDown)
        {
            fixedSize = fixedSize.scaled(getMaxPreviewSize(), Qt::KeepAspectRatio);
        }

        const auto shouldScaleUp =
            (fixedSize.width() < getMinPreviewSize().width()) &&
            (fixedSize.height() < getMinPreviewSize().height());
        if (shouldScaleUp)
        {
            fixedSize = fixedSize.scaled(getMinPreviewSize(), Qt::KeepAspectRatio);
        }

        return fixedSize;
    }

    QRect PreviewContentWidget::getPreviewScaledRect() const
    {
        QPoint topLeft(0, 0);

        if (TextControl_)
        {
            topLeft.ry() +=
                getTextBubbleSize().height() +
                getTextBottomMargin();
        }

        QRect result(
            QPoint(0, 0),
            getPreviewScaledSizeF().toSize()
        );

        result.moveTopLeft(topLeft);

        return result;
    }

    const QSizeF& PreviewContentWidget::getPreviewScaledSizeF() const
    {
        if (!PreviewScaledSize_.isValid())
        {
            PreviewScaledSize_ = evaluatePreviewScaledSize(width());
            assert(PreviewScaledSize_.isValid());
        }

        return PreviewScaledSize_;
    }

    const QPainterPath& PreviewContentWidget::getTextBubble() const
    {
        const auto textBubbleEmpty = TextBubble_.isEmpty();
        if (!textBubbleEmpty)
        {
            return TextBubble_;
        }

        const auto &textSize = getTextSize();
        if (textSize.isEmpty())
        {
            return TextBubble_;
        }

        const auto textBubbleSize = getTextBubbleSize();

        TextBubble_ = Utils::renderMessageBubble(
            textBubbleSize,
            getTextBubbleBorderRadius(),
            isOutgoing()
        );

        assert(!TextBubble_.isEmpty());

        return TextBubble_;
    }

    QSize PreviewContentWidget::getTextSize() const
    {
        if (!TextControl_)
        {
            TextSize_ = QSize(0, 0);
        }

        if (TextSize_.isValid())
        {
            return TextSize_;
        }

        return TextControl_->getTextSize();
    }

    QSize PreviewContentWidget::getTextBubbleSize() const
    {
        auto bubbleSize = getTextSize();

        const auto textHeight = bubbleSize.height();

        bubbleSize.setHeight(
            std::max(
                Utils::scale_value(32),
                textHeight
            )
        );

        bubbleSize.setHeight(
            Utils::applyMultilineTextFix(textHeight, bubbleSize.height())
        );

        bubbleSize.rwidth() += getBubbleHorPadding();
        bubbleSize.rwidth() += getBubbleHorPadding();
        bubbleSize.rheight() += getBubbleVertPadding();

        return bubbleSize;
    }

    void PreviewContentWidget::limitPreviewSize()
    {
        assert(!Preview_.isNull());
        assert(!PreviewGenuineSize_.isEmpty());

        const auto previewSize = PreviewGenuineSize_;
        const auto shouldScalePreviewDown =
            (previewSize.width() > getMaxPreviewSize().width()) ||
            (previewSize.height() > getMaxPreviewSize().height());
        if (!shouldScalePreviewDown)
        {
            return;
        }

        Utils::check_pixel_ratio(Preview_);

        const auto scaledSize = Utils::scale_bitmap(getMaxPreviewSize().toSize());

        auto task = new ResizePixmapTask(Preview_, scaledSize);

        const auto succeed = QObject::connect(
            task, &ResizePixmapTask::resizedSignal,
            this, &PreviewContentWidget::onPreviewSizeLimited
        );
        assert(succeed);

        QThreadPool::globalInstance()->start(task);
    }

    void PreviewContentWidget::invalidateSizes()
    {
        // invalidate size-dependent children

        ClippingPath_ = QPainterPath();

        PreviewScaledSize_ = QSize();
        assert(!PreviewScaledSize_.isValid());

        TextSize_ = QSize();
        assert(!TextSize_.isValid());

        TextBubble_ = QPainterPath();
        assert(TextBubble_.isEmpty());
    }

    void PreviewContentWidget::prepareTextGeometry()
    {
        if (!TextControl_)
        {
            return;
        }

        const auto documentWidth = (
            width() -
            getBubbleHorPadding() -
            getBubbleHorPadding()
        );

        const auto textWidthChanged = (TextControl_->width() != documentWidth);
        if (textWidthChanged)
        {
            TextControl_->document()->setTextWidth(documentWidth);
            TextControl_->setFixedWidth(documentWidth);
        }

        TextControl_->move(
            getBubbleHorPadding(),
            getBubbleVertPadding()
        );
    }

    void PreviewContentWidget::renderPreloader(QPainter &p)
    {
        renderPreloaderBubble(p);

        if (!isPreloaderVisible())
        {
            return;
        }

        p.save();

        auto icon = Themes::GetPixmap(Themes::PixmapResourceId::FileSharingNoImageIcon);

        auto targetRect = icon->GetRect();
        targetRect.moveCenter(
            getPreviewScaledRect().center()
        );

        icon->Draw(p, targetRect);

        p.restore();
    }

    void PreviewContentWidget::renderPreloaderBubble(QPainter &p)
    {
        p.save();

        p.setBrush(Qt::white);

        const auto borderRadius = Utils::scale_value(8);
        p.drawRoundedRect(getPreviewScaledRect(), borderRadius, borderRadius);

        p.restore();
    }

    namespace
    {
        QBrush getBodyBrush(const bool isOutgoing, const bool isSelected)
        {
            QLinearGradient grad(0, 0, 1, 0);

            grad.setCoordinateMode(QGradient::ObjectBoundingMode);

            if (isSelected)
            {
                const auto color0 = (
                    isOutgoing ?
                    QColor(0x57, 0x9e, 0x1c, (int32_t)(0.9 * 255)) :
                    QColor(0x57, 0x9e, 0x1c, (int32_t)(1.0 * 255))
                    );
                grad.setColorAt(0, color0);

                const auto color1 = (
                    isOutgoing ?
                    QColor(0x57, 0x9e, 0x1c, (int32_t)(0.72 * 255)) :
                    QColor(0x57, 0x9e, 0x1c, (int32_t)(0.72 * 255))
                    );
                grad.setColorAt(1, color1);
            }
            else
            {
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
            }

            QBrush result(grad);
            result.setColor(QColor(0, 0, 0, 0));

            assert(!result.isOpaque());
            return result;
        }

        int32_t getBubbleHorPadding()
        {
            return Utils::scale_value(16);
        }

        int32_t getBubbleVertPadding()
        {
            return Utils::scale_value(5);
        }

        const QSizeF& getMaxPreviewSize()
        {
            static const QSizeF size(
                Utils::scale_value(640),
                Utils::scale_value(320)
            );

            return size;
        }

        const QSizeF& getMinPreviewSize()
        {
            static const QSizeF size(
                Utils::scale_value(48),
                Utils::scale_value(48)
            );

            return size;
        }

        const QFont& getPreviewTextFont()
        {
            static QFont font(
                Utils::appFontFamily(Utils::FontsFamily::SEGOE_UI)
            );

            return font;
        }

        int32_t getTextBottomMargin()
        {
            return Utils::scale_value(2);
        }

        int32_t getTextBubbleBorderRadius()
        {
            return Utils::scale_value(12);
        }
    }
}
