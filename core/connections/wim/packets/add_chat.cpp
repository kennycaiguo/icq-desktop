#include "stdafx.h"
#include "add_chat.h"

#include "../../../http_request.h"
#include "../../../tools/system.h"

using namespace core;
using namespace wim;

add_chat::add_chat(
	const wim_packet_params& _params,
	const std::string& _m_chat_name,
	const std::string& _m_chat_members)
	:	wim_packet(_params),
	m_chat_name(_m_chat_name),
	m_chat_members(_m_chat_members)
{
}

add_chat::~add_chat()
{
}

int32_t add_chat::init_request(std::shared_ptr<core::http_request_simple> _request)
{
	std::stringstream ss_url;
	
	ss_url << c_wim_host << "mchat/AddChat" <<
		"?f=json" <<
		"&chat_name="<< escape_symbols(m_chat_name) <<
        "&aimsid=" << get_params().aimsid_ <<
        "&r=" <<  core::tools::system::generate_guid() <<
		"&members=" << m_chat_members;

	_request->set_url(ss_url.str());
    _request->set_keep_alive();

	return 0;
}

int32_t add_chat::parse_response_data(const rapidjson::Value& _data)
{
	return 0;
}
