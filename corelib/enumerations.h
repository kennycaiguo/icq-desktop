#pragma once

namespace core
{

    enum class profile_state
    {
        min = 0,

        online,
        dnd,
        invisible,

        max,
    };

	enum class message_type
	{
		min,

        undefined,
		base,
		file_sharing,
		sms,
		sticker,
		chat_event,
        voip_event,

		max
	};

	enum class preview_content_type
	{
		invalid,

		image,

		min = image,
		max = image
	};

	enum class file_sharing_function
	{
		min,

		download_file,
		download_meta,
		check_local_copy_exists,

		max
	};

	enum class sticker_size
	{
		min,

		small,
		medium,
		large,

		max
	};

	enum class chat_event_type
	{
		invalid,

		min,

		added_to_buddy_list,

		mchat_add_members,
		mchat_invite,
		mchat_leave,
		mchat_del_members,
		mchat_kicked,

		chat_name_modified,

		buddy_reg,
		buddy_found,

		birthday,

		max
	};

    enum class voip_event_type
    {
        invalid,

        min,

        missed_call,
        call_ended,
        accept,

        max
    };

	inline std::ostream& operator<<(std::ostream &oss, const message_type arg)
	{
		switch(arg)
		{
			case message_type::base:
				oss << "base";
				break;

			case message_type::file_sharing:
				oss << "file_sharing";
				break;

			case message_type::sticker:
				oss << "sticker";
				break;

			case message_type::sms:
				oss << "sms";
				break;

			default:
				assert(!"unknown core::message_type value");
				break;
		}

		return oss;
	}

	inline std::ostream& operator<<(std::ostream &oss, const file_sharing_function arg)
	{
		assert(arg > file_sharing_function::min);
		assert(arg < file_sharing_function::max);

		switch(arg)
		{
			case file_sharing_function::check_local_copy_exists:
				oss << "check_local_copy_exists";
				break;

			case file_sharing_function::download_file:
				oss << "download_file";
				break;

			case file_sharing_function::download_meta:
				oss << "download_meta";
				break;

			default:
				assert(!"unknown core::file_sharing_function value");
				break;
		}

		return oss;
	}

	inline std::ostream& operator<<(std::ostream &oss, const sticker_size arg)
	{
		assert(arg > sticker_size::min);
		assert(arg < sticker_size::max);

		switch(arg)
		{
		case sticker_size::small:
			oss << "small";
			break;

		case sticker_size::medium:
			oss << "medium";
			break;

		case sticker_size::large:
			oss << "large";
			break;

		default:
			assert(!"unknown core::sticker_size value");
			break;
		}

		return oss;
	}

    inline std::wostream& operator<<(std::wostream &oss, const sticker_size arg)
    {
        assert(arg > sticker_size::min);
        assert(arg < sticker_size::max);

        switch(arg)
        {
        case sticker_size::small:
            oss << L"small";
            break;

        case sticker_size::medium:
            oss << L"medium";
            break;

        case sticker_size::large:
            oss << L"large";
            break;

        default:
            assert(!"unknown core::sticker_size value");
            break;
        }

        return oss;
    }

    enum class group_chat_info_errors
    {
        min = 0,
        not_in_chat = 1,

        max,
    };

    namespace stats
    {
        enum class stats_event_names
        {
            min = 0,
            service_session_start,
            start_session,

            // registration
            reg_page_phone,
            reg_login_phone,
            reg_page_uin,
            reg_login_uin,

            // main window
            main_window_fullscreen,
            main_window_close,
            main_window_minimize,
            main_window_resize,

            // group chat
            groupchat_from_create_button,
            groupchat_from_dialog,
            groupchat_created,
            groupchat_create_rename,
            groupchat_create_members_count,
            groupchat_members_count,
            groupchat_leave,
            livechat_leave,

            // filesharing
            filesharing_sent,
            filesharing_sent_image,
            filesharing_sent_success,
            filesharing_count,
            filesharing_filesize,
            filesharing_dnd_recents,
            filesharing_dnd_dialog,
            filesharing_cancel,
            filesharing_incoming,
            filesharing_incoming_image,

            //


            max,
        };

        inline std::ostream& operator<<(std::ostream &oss, const stats_event_names arg)
        {
            assert(arg > stats_event_names::min);
            assert(arg < stats_event_names::max);

            switch(arg)
            {
            case stats_event_names::start_session : oss << "Start_Session [session enable]"; break;
            case stats_event_names::service_session_start : assert(false); break;

            // registration
            case stats_event_names::reg_page_phone : oss << "Reg_Page_Phone"; break;
            case stats_event_names::reg_login_phone : oss << "Reg_Login_Phone"; break;
            case stats_event_names::reg_page_uin : oss << "Reg_Page_Uin"; break;
            case stats_event_names::reg_login_uin : oss << "Reg_Login_UIN"; break;

            // main window
            case stats_event_names::main_window_fullscreen : oss << "Mainwindow_Fullscreen"; break;
            case stats_event_names::main_window_close : oss << "Mainwindow_Close"; break;
            case stats_event_names::main_window_minimize : oss << "Mainwindow_Minimize"; break;
            case stats_event_names::main_window_resize : oss << "Mainwindow_Resize"; break;

            // group chat
            case stats_event_names::groupchat_from_create_button : oss << "Groupchat_FromCreateButton"; break;
            case stats_event_names::groupchat_from_dialog : oss << "Groupchat_FromDialog"; break;
            case stats_event_names::groupchat_created : oss << "Groupchat_Created"; break;
            case stats_event_names::groupchat_create_rename : oss << "Groupchat_Create_Rename"; break;
            case stats_event_names::groupchat_create_members_count : oss << "Groupchat_Create_MembersCount"; break;
            case stats_event_names::groupchat_members_count : oss << "Groupchat_MembersCount"; break;
            case stats_event_names::groupchat_leave : oss << "Groupchat_Leave"; break;
            case stats_event_names::livechat_leave : oss << "Livechat_Leave"; break;

            // filesharing
            case stats_event_names::filesharing_sent : oss << "Filesharing_Sent"; break;
            case stats_event_names::filesharing_sent_image : oss << "Filesharing_Sent_Image"; break;
            case stats_event_names::filesharing_sent_success : oss << "Filesharing_Sent_Success"; break;
            case stats_event_names::filesharing_count : oss << "Filesharing_Count"; break;
            case stats_event_names::filesharing_filesize : oss << "Filesharing_Filesize"; break;
            case stats_event_names::filesharing_dnd_recents : oss << "Filesharing_DNDRecents"; break;
            case stats_event_names::filesharing_dnd_dialog : oss << "Filesharing_DNDDialog"; break;
            case stats_event_names::filesharing_cancel : oss << "Filesharing_Cancel"; break;
            case stats_event_names::filesharing_incoming : oss << "Filesharing_Incoming"; break;
            case stats_event_names::filesharing_incoming_image : oss << "Filesharing_Incoming_Image"; break;


            default:
                assert(!"unknown core::stats_event_names ");
                break;
            }

            return oss;
        }
    }


}