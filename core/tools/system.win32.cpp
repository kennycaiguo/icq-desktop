#include "stdafx.h"

#include "system.h"
#include "win32/dll.h"
#include "../profiling/profiler.h"

#pragma comment(lib, "Rpcrt4.lib")

namespace fs = boost::filesystem;

CORE_TOOLS_SYSTEM_NS_BEGIN

namespace
{
	std::wstring get_user_downloads_dir_xp();

	std::wstring get_user_downloads_dir_vista();
}

unsigned long get_current_thread_id()
{
	return ::GetCurrentThreadId();
}

bool is_dir_writable(const std::wstring &_dir_path_str)
{
    const fs::wpath dir_path(_dir_path_str);

    const auto is_dir = fs::is_directory(dir_path);
    assert(is_dir);

    if (!is_dir)
    {
        return false;
    }

    const auto test_path = (dir_path / generate_guid());

    {
        std::ofstream out(test_path.wstring());
        if (out.fail())
        {
            return false;
        }
    }

    fs::remove(test_path);

    return true;
}

bool move_file(const std::wstring& _old_file, const std::wstring& _new_file)
{
	return !!::MoveFileEx(_old_file.c_str(), _new_file.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
}

std::wstring get_user_profile()
{
	wchar_t buffer[MAX_PATH + 1];

	const auto error = ::SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, buffer);
	if (FAILED(error))
	{
		return std::wstring();
	}

	return buffer;
}

std::string generate_guid()
{
	std::string guid_string;

	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	guid_string = boost::lexical_cast<std::string>(uuid);

	return guid_string;
}

std::string generate_internal_id()
{
    static int internal_id = 0;
    std::stringstream guid_string;

    guid_string << generate_guid() << "-" << ++internal_id; 

    return guid_string.str();
}

std::wstring get_user_downloads_dir()
{
	static std::wstring cached_path;

	if (!cached_path.empty())
	{
		return cached_path;
	}

	cached_path = get_user_downloads_dir_vista();

	if (!cached_path.empty())
	{
		return cached_path;
	}

	cached_path = get_user_downloads_dir_xp();

	return cached_path;
}

std::string to_upper(std::string str)
{
    return boost::locale::to_upper(str);
}

namespace
{
	std::wstring get_user_downloads_dir_xp()
	{
		WCHAR path[MAX_PATH] = { 0 };

		const auto error = ::SHGetFolderPath(nullptr, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, nullptr, 0, Out path);
		if (FAILED(error))
		{
			return std::wstring();
		}

		assert(fs::is_directory(path));
		return path;
	}

	std::wstring get_user_downloads_dir_vista()
	{
		PWSTR path = nullptr;

		static auto proc = tools::win32::import_proc<decltype(&::SHGetKnownFolderPath)>(L"Shell32.dll", "SHGetKnownFolderPath");
		if (!proc)
		{
			return std::wstring();
		}

		const auto error = proc->get()(FOLDERID_Downloads, 0, nullptr, Out &path);
		if (FAILED(error))
		{
			return std::wstring();
		}

		std::wstring result(path);
		assert(fs::is_directory(result));

		::CoTaskMemFree(path);

		return result;
	}
}

CORE_TOOLS_SYSTEM_NS_END