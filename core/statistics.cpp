#include "stdafx.h"
#include "statistics.h"

#include "core.h"
#include "tools/binary_stream.h"
#include "tools/strings.h"
#include "tools/system.h"
#include "tools/tlv.h"
#include "../external/curl/include/curl.h"
#include "http_request.h"
#include "tools/hmac_sha_base64.h"
#include "async_task.h"
#include "utils.h"
#include "../corelib/enumerations.h"

using namespace core::stats;
using namespace core;

enum statistics_info_types
{
    last_sent_time = 0,
    event_name = 1,
    event_time = 2,
    event_id = 3
};

long long statistics::stats_event::session_event_id_ = 0;

statistics::statistics(const std::wstring& _file_name, const std::string& _user_key)
    : file_name_(_file_name)
    , changed_(false)
    , stats_thread_(new async_executer())
    , user_key_(_user_key)
    , last_sent_time_(std::chrono::system_clock::now())
{
}

void statistics::init()
{
    load();
    start_save();
    start_send();
}

statistics::~statistics()
{
    if (save_timer_ > 0 && g_core)
        g_core->stop_timer(save_timer_);

    if (send_timer_ > 0 && g_core)
        g_core->stop_timer(send_timer_);

    save_if_needed();
    
    stats_thread_.reset();
}

void statistics::start_save()
{
    std::weak_ptr<statistics> wr_this = shared_from_this();

    save_timer_ =  g_core->add_timer([wr_this]
    {
        auto ptr_this = wr_this.lock();
        if (!ptr_this)
            return;

        ptr_this->save_if_needed();
    }, save_to_file_interval_ms);
}

void statistics::start_send()
{
    auto current_time = std::chrono::system_clock::now();
    if (current_time - last_sent_time_ >= std::chrono::milliseconds(send_interval_ms))
        send_async();

    std::weak_ptr<statistics> wr_this = shared_from_this();

    send_timer_ = g_core->add_timer([wr_this]
    {
        auto ptr_this = wr_this.lock();
        if (!ptr_this)
            return;

        ptr_this->send_async();
    }, send_interval_ms);
}

bool statistics::load()
{
    core::tools::binary_stream bstream;
    if (!bstream.load_from_file(file_name_))
        return false;

    return unserialize(bstream);
}

void statistics::serialize(tools::binary_stream& _bs) const
{
    tools::tlvpack pack;
    int32_t counter = 0;

    // push stats info
    {
        tools::tlvpack value_tlv;
        value_tlv.push_child(tools::tlv(statistics_info_types::last_sent_time, std::chrono::system_clock::to_time_t(last_sent_time_)));

        tools::binary_stream bs_value;
        value_tlv.serialize(bs_value);
        pack.push_child(tools::tlv(++counter, bs_value));
    }

    for (auto stat_event = events_.begin(); stat_event != events_.end(); ++stat_event)
    {
        tools::tlvpack value_tlv;
        // TODO : push id, time, ..
        value_tlv.push_child(tools::tlv(statistics_info_types::event_name, stat_event->get_name()));
        value_tlv.push_child(tools::tlv(statistics_info_types::event_time, std::chrono::system_clock::to_time_t(stat_event->get_time())));
        value_tlv.push_child(tools::tlv(statistics_info_types::event_id, stat_event->get_id()));

        tools::binary_stream bs_value;
        value_tlv.serialize(bs_value);
        pack.push_child(tools::tlv(++counter, bs_value));
    }

    pack.serialize(_bs);
}

bool statistics::unserialize(tools::binary_stream& _bs)
{
    if (!_bs.available())
    {
        assert(false);
        return false;
    }

    tools::tlvpack pack;
    if (!pack.unserialize(_bs))
        return false;

    int counter = 0;
    for (auto tlv_val = pack.get_first(); tlv_val; tlv_val = pack.get_next())
    {
        tools::binary_stream val_data = tlv_val->get_value<tools::binary_stream>();

        tools::tlvpack pack_val;
        if (!pack_val.unserialize(val_data))
            return false;

        if (counter++ == 0)
        {
            auto tlv_last_sent_time = pack_val.get_item(statistics_info_types::last_sent_time);

            if (!tlv_last_sent_time)
            {
                assert(false);
                return false;
            }
            
            time_t last_time = tlv_last_sent_time->get_value<time_t>();
            last_sent_time_ = std::chrono::system_clock::from_time_t(last_time);
        }
        else
        {
            auto curr_event_name = pack_val.get_item(statistics_info_types::event_name);

            if (!curr_event_name)
            {
                assert(false);
                return false;
            }
            stats_event_names name = curr_event_name->get_value<stats_event_names>();

            auto tlv_event_time = pack_val.get_item(statistics_info_types::event_time);
            if (!tlv_event_time)
            {
                assert(false);
                return false;
            }

            auto tlv_event_id = pack_val.get_item(statistics_info_types::event_id);
            if (!tlv_event_id)
            {
                assert(false);
                return false;
            }
            auto read_event_time = std::chrono::system_clock::from_time_t(tlv_event_time->get_value<int>());
            auto read_event_id = tlv_event_id->get_value<int>();
            insert_event(name, read_event_time, read_event_id);
            // events_.push_back(stats_event(name));
        }
    }

    return true;
}

void statistics::save_if_needed()
{
    if (changed_)
    {
        changed_ = false;

        auto bs_data = std::make_shared<tools::binary_stream>();
        serialize(*bs_data);
        std::wstring file_name = file_name_;

        g_core->save_async([bs_data, file_name]
        { 
            bs_data->save_2_file(file_name);
            return 0;
        });
    }
}

void statistics::clear()
{
    last_sent_time_ = std::chrono::system_clock::now();

    auto last_service_event_ptr = events_.end();
    while (last_service_event_ptr != events_.begin())
    {
        --last_service_event_ptr;
        if (last_service_event_ptr->get_name() == stats_event_names::service_session_start)
            break;
    }

    auto last_service_event = *last_service_event_ptr;
    events_.clear();
    events_.push_back(last_service_event);

    changed_ = true;

    // reset_session_event_id();
    // TODO : mb need save map with counts here?
}

void statistics::send_async()
{
    if (events_.empty())
        return;

    std::vector<std::string> post_data_vector = get_post_data();
    std::weak_ptr<statistics> wr_this = shared_from_this();

    if (post_data_vector.empty())
        return;

    stats_thread_->run_async_function([post_data_vector]
    {
        for (auto& post_data : post_data_vector)
            statistics::send(post_data);
        return 0;
            
    })->on_result_ = [wr_this](int32_t _error)
    {
        auto ptr_this = wr_this.lock();
        if (!ptr_this)
            return;
        ptr_this->clear();
        ptr_this->save_if_needed();
    };
}

std::string statistics::events_to_json(events_ci begin, events_ci end) const
{
    std::stringstream data_stream;
    std::map<stats_event_names, int> events_and_count;

    for (auto stat_event  = begin; stat_event != end; ++stat_event)
    {
        if (stat_event != begin) 
            data_stream << ",";
        data_stream << stat_event->to_string();
        ++events_and_count[stat_event->get_name()];
    }

    data_stream << "],\"bm\":false,\"bn\":{";

    for (auto stat_event  = events_and_count.begin(); stat_event != events_and_count.end(); ++stat_event)
    {
        if (stat_event != events_and_count.begin()) 
            data_stream << ",";
        data_stream << "\"" << stat_event->first << "\":" << stat_event->second;
    }
    return data_stream.str();
}

std::vector<std::string> statistics::get_post_data() const
{
    std::vector<std::string> result;

    events_ci begin = events_.begin();

    while (begin != events_.end())
    {
        events_ci end = std::next(begin);
        while (end != events_.end() && end->get_name() != stats_event_names::service_session_start)
            ++end;

        assert(begin->get_name() == stats_event_names::service_session_start);
        
        const long long time_now = std::chrono::system_clock::to_time_t(begin->get_time()) * 1000; // milliseconds
        ++begin;
        
        if (begin == end)
            continue;

        auto time2 = time_now;
        auto bb = 0;
        auto time3 = time_now - bb;
        auto bq = 0;

        auto version = core::utils::get_user_agent();
        std::stringstream data_stream;

        data_stream << "{\"a\":{\"af\":" << time_now
            <<",\"aa\":1,\"ab\":10,\"ac\":9,\"ae\":\""<< version 
            << "\",\"ad\":\"" << flurry_key 
            << "\",\"ag\":" << time2 
            << ",\"ah\":" << time3
            << ",\"ak\":1," 
            << "\"cg\":\"" << user_key_
            << "\"},\"b\":[{\"bd\":\"\",\"be\":\"\",\"bk\":-1,\"bl\":0,\"bj\":\"ru\",\"bo\":[";

        data_stream << events_to_json(begin, end);

        data_stream << "}"
            <<",\"bv\":[],\"bt\":false,\"bu\":{},\"by\":[],\"cd\":0,"
            << "\"ba\":" << time3
            << ",\"bb\":" << bb
            << ",\"bc\":-1,\"ch\":\"Etc/GMT-3\"}]}";
        result.push_back(data_stream.str());

        if (end != events_.end())
        {
            begin = end;
        }
        else
            break;
    }

    return result;
}

bool statistics::send(const std::string& post_data)
{
    core::proxy_settings ps;
    core::http_request_simple post_request(ps);
    post_request.set_connect_timeout(1000);
    post_request.set_timeout(1000);
    post_request.set_keep_alive();

    auto result_url = flurry_url 
        + "?d=" + core::tools::base64::encode64(post_data) 
        + "&c=" + core::tools::adler32(post_data);
    post_request.set_url(result_url);
    return post_request.get();
}

void statistics::insert_event(stats_event_names _event_name, std::chrono::system_clock::time_point _event_time, int _event_id)
{
    events_.emplace_back(_event_name, _event_time, _event_id);
    changed_ = true;
}

void statistics::insert_event(stats_event_names _event_name)
{
    if (_event_name == stats_event_names::start_session)
        insert_event(core::stats::stats_event_names::service_session_start);
    insert_event(_event_name, std::chrono::system_clock::now(), -1);
}

statistics::stats_event::stats_event(stats_event_names _name, std::chrono::system_clock::time_point _event_time, int _event_id)
    : name_(_name)
    , event_time_(_event_time)
{
    if (_event_id == -1)
        event_id_ = session_event_id_++; // started from 1
    else
        event_id_ = _event_id;
}

const std::string statistics::stats_event::to_string() const
{
    std::stringstream result;

    // TODO : use actual params here
    auto bq = 3001;
    auto br = 0;

    std::stringstream params_in_json;
    for (auto prop = props.begin(); prop != props.end(); ++prop)
    {
        if (prop != props.begin())
            params_in_json << ",";
        params_in_json << "\"" << prop->first << "\",\"" << prop->second << "\"";
    }

    result << "{\"ce\":" << event_id_
        << ",\"bp\":\"" << name_ 
        << "\",\"bq\":" << bq
        << ",\"bs\":{" << params_in_json.str() <<"},"
        << "\"br\":" << br << "}";
    return result.str();
}

stats_event_names statistics::stats_event::get_name() const
{
    return name_;
}

void statistics::stats_event::reset_session_event_id()
{
    session_event_id_ = 0;
}

std::chrono::system_clock::time_point statistics::stats_event::get_time() const
{
    return event_time_;
}

int statistics::stats_event::get_id() const
{
    return event_id_;
}