#pragma once

#include "../../../namespaces.h"

CORE_WIM_NS_BEGIN

class im;

class fetch_event
{
public:
	virtual int32_t parse(const rapidjson::Value& node_event_data) { return  0; };

	virtual void on_im(std::shared_ptr<core::wim::im> _im) {};

};

CORE_WIM_NS_END