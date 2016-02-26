#pragma once

namespace Themes
{
	enum class PixmapResourceId
	{
		Invalid,
		Min,

		ContactListSendingMark,
		ContactListReadMark,
		ContactListDeliveredMark,
		ContactListAddContact,
		ContactListAddContactHovered,

		FileSharingDownload,
		FileSharingPlainCancel,
		FileSharingFileTypeIconUnknown,
        FileSharingNoImageIcon,

		PreviewerClose,
		ContentMuteNotify,
		ContentMuteNotifyNew,

        VoipEventMissedIcon,
        VoipEventIncomingCallIcon,
        VoipEventOutgoingCallIcon,
        VoipEventCallEndedIcon,

		Max
	};
}