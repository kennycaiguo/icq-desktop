#include "stdafx.h"
#include "send_feedback.h"

#include "../../../log/log.h"
#include "../../../http_request.h"
#include "../../../corelib/enumerations.h"
#include "../../../tools/system.h"

using namespace core;
using namespace wim;

send_feedback::send_feedback(const wim_packet_params& _params, const std::string &url, const std::map<std::string, std::string>& fields, const std::vector<std::string>& attachments)
    :
    wim_packet(_params),
    url_(url)
{
    fields_.insert(fields.begin(), fields.end());
    attachments_.assign(attachments.begin(), attachments.end());
    
    if (fields_.find("fb.user_name") == fields_.end() || fields_["fb.user_name"].empty())
        fields_["fb.user_name"] = _params.aimid_;
    
    fields_["fb.question.3003"] = _params.aimid_;
}

send_feedback::~send_feedback()
{
}

int32_t send_feedback::init_request(std::shared_ptr<core::http_request_simple> _request)
{
    auto from = log::get_net_file_path();
    auto to = log::get_net_file_path().parent_path().append("feedbacklog.txt");
    boost::filesystem::copy_file(from, to, boost::filesystem::copy_option::overwrite_if_exists);
    auto fsize = boost::filesystem::file_size(to);
    const size_t max_log_size = (1024 * 1024); // 1 Mb per log file
    if (fsize > max_log_size)
    {
        std::ifstream ifs(to.string(), std::ios::binary);
        std::vector<char> data;
        data.resize(max_log_size);
        ifs.seekg(-((std::fstream::off_type)data.size()), std::ios::end);
        ifs.read(&data[0], data.size());
        ifs.close();
        
        boost::filesystem::remove(to);
        
        std::ofstream ofs(to.string(), std::ios::binary);
        ofs.write(&data[0], data.size());
        ofs.close();
    }
    for (auto f: fields_)
        _request->push_post_form_parameter(f.first, f.second);
    for (auto a: attachments_) 
        _request->push_post_form_file("fb.attachement", tools::wstring_to_string(tools::from_utf8(a)));
    _request->push_post_form_file("fb.attachement", tools::wstring_to_string(tools::from_utf8(to.string())));
    _request->push_post_form_parameter("submit", "send");
    _request->push_post_form_parameter("r", core::tools::system::generate_guid());

    _request->set_url(url_);
    _request->set_post_form(true);

    return 0;
}

int32_t send_feedback::parse_response(std::shared_ptr<core::tools::binary_stream> response)
{
    return 0;
}

int32_t send_feedback::execute_request(std::shared_ptr<core::http_request_simple> request)
{
    if (!request->post())
        return wpie_network_error;
    
    http_code_ = (uint32_t)request->get_response_code();
    if (http_code_ != 200)
        return wpie_http_error;
    
    auto to = log::get_net_file_path().parent_path().append("feedbacklog.txt");
    boost::filesystem::remove(to);

    return 0;
}
