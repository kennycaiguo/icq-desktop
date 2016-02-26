#ifndef __VOIP_TOOLS_H__
#define __VOIP_TOOLS_H__

namespace voipTools {
static void __showBounds(QPainter& painter, const QRect& rc) {
    const QPen wasPen = painter.pen();
    painter.setPen  (QColor(0, 255, 0, 255));
    painter.fillRect(QRect(rc.left(), rc.top(), rc.width() - 1, rc.height() - 1), QColor(0, 0, 255, 100));
    painter.drawRect(QRect(rc.left(), rc.left(), rc.width() - 1, rc.height() - 1));
    painter.setPen(wasPen);
}
}

#if defined(_DEBUG) && !defined(__APPLE__)
    #define DISPLAY_BOUNDS(painter) voipTools::__showBounds((painter), rect())
#else
    #define DISPLAY_BOUNDS(painter) (void)0
#endif

#endif//__VOIP_TOOLS_H__