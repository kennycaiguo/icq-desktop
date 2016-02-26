#pragma once

#include "../../../corelib/iserializable.h"

namespace core
{
    struct icollection;

    enum class voip_event_type;
}

namespace Themes
{
    typedef std::shared_ptr<class IThemePixmap> IThemePixmapSptr;
}

namespace HistoryControl
{

    using namespace core;

    typedef std::shared_ptr<class VoipEventInfo> VoipEventInfoSptr;

    class VoipEventInfo : public core::iserializable
    {
    public:
        static VoipEventInfoSptr Make(const coll_helper &info);

        virtual void serialize(Out coll_helper &_coll) const override;

        virtual bool unserialize(const coll_helper &_coll) override;

        QString formatEventText() const;

        const QString& getContactAimid() const;

        const QString& getContactFriendly() const;

        bool isClickable() const;

        bool isIncomingCall() const;

        bool isVisible() const;

        Themes::IThemePixmapSptr loadIcon(const bool isHovered) const;

    private:
        VoipEventInfo();

        QString ContactAimid_;

        QString ContactFriendly_;

        voip_event_type Type_;

        int32_t DurationSec_;

        bool IsIncomingCall_;

    };

}