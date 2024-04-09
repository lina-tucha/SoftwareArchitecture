#pragma once

#include <string>
#include <vector>
#include <optional>

#include <Poco/JSON/Object.h>

namespace database
{
    class Post{
        private:
            long _id;
            long _user_id;
            std::string _text;
            std::string _date;

        public:

            static Post  fromJSON(const std::string & str);

            long               get_id() const;
            long               get_user_id() const;
            const std::string &get_text() const;
            const std::string &get_date() const;

            long&        id();
            long&        user_id();
            std::string& text();
            std::string& date();

            static std::optional<Post> read_by_id(long id);
            static std::vector<Post> read_by_user_id(long user_id);
            void   add();
            void   update();
            Poco::JSON::Object::Ptr toJSON() const;

    };
}