#pragma once

#include <string>
#include <vector>
#include <optional>

#include <Poco/JSON/Object.h>

namespace database
{
    class Message{
        private:
            long _id;
            long _sender_id;
            long _recipient_id;
            std::string _text;
            std::string _date;

        public:

            static Message fromJSON(const std::string & str);

            long               get_id() const;
            long               get_sender_id() const;
            long               get_recipient_id() const;
            const std::string &get_text() const;
            const std::string &get_date() const;

            long&        id();
            long&        sender_id();
            long&        recipient_id();
            std::string& text();
            std::string& date();

            static std::optional<Message> read_by_id(long id);
            static std::vector<Message> read_by_sender_id(long sender_id);
            static std::vector<Message> read_by_recipient_id(long recipient_id);
            void   add();
            void   update();
            Poco::JSON::Object::Ptr toJSON() const;

    };
}