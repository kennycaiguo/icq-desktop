#pragma once
#ifdef _WIN32

namespace core
{
    class async_executer;

    namespace dump
    {
        const static std::string app_id = "517065520bb44e84bf2912da3b74b61f";
        const static std::string hockeyapp_url =  "https://rink.hockeyapp.net/api/2/apps/" + app_id + "/crashes/upload";

        class report_sender : public std::enable_shared_from_this<report_sender>
        {
        public:
            report_sender(const std::string& _login);
            void send_report();
            ~report_sender();
        private:
            void clear_report_folder();
            bool is_report_existed();
            bool send_to_hockey_app(const std::string& _login);
            std::unique_ptr<async_executer> send_thread_;
            std::string login_;
        };
    }
}

#endif // _WIN32
