#pragma once

namespace core
{
    class coll_helper;
    class async_executer;

    namespace tools
    {
        class binary_stream;
    }

    // TODO : change it after testing
    const static std::string flurry_key = "KYVQVW38PB2SP8CRJJ9R"; // test ICQ 
    const static std::string flurry_url = "https://data.flurry.com/aah.do";

    const static uint32_t send_interval_ms = 1000 * 60 * 60; // 6 hours for release, 1 for testing time
    const static uint32_t save_to_file_interval_ms = 1000 * 10; // 10 seconds

    namespace stats
    {
        enum class stats_event_names;
        
        class statistics : public std::enable_shared_from_this<statistics>
        {
        private:

            class stats_event
            {
            public:
                const std::string to_string() const;
                stats_event(stats_event_names _name, std::chrono::system_clock::time_point _event_time, int _event_id);
                stats_event_names get_name() const;
                void reset_session_event_id();
                std::chrono::system_clock::time_point get_time() const;
                int get_id() const;
            private:
                stats_event_names name_;
                int event_id_; // natural serial number, starting from 1
                std::vector<std::pair<std::string, std::string> > props;
                static long long session_event_id_;
                std::chrono::system_clock::time_point event_time_;
            };

            typedef std::vector<stats_event>::const_iterator events_ci;
            std::map<std::string, tools::binary_stream> values_;
            std::wstring file_name_;

            bool changed_;
            uint32_t save_timer_;
            uint32_t send_timer_;
            std::string user_key_;
            std::unique_ptr<async_executer> stats_thread_;
            std::vector<stats_event> events_;
            std::string events_to_json(events_ci begin, events_ci end) const;
            std::chrono::system_clock::time_point last_sent_time_;
            std::vector<std::string> get_post_data() const;
            static bool send(const std::string& post_data);

            void serialize(tools::binary_stream& _bs) const;
            bool unserialize(tools::binary_stream& _bs);
            void save_if_needed();
            void send_async();
            bool load();
            void start_save();
            void start_send();
            void insert_event(stats_event_names _event_name, std::chrono::system_clock::time_point _event_time, int _event_id);
            void clear();
        public:
            statistics(const std::wstring& _file_name, const std::string& _user_key);
            virtual ~statistics();

            void init();
            void insert_event(stats_event_names _event_name);
        };
    }
}