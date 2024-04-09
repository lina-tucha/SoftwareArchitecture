#include "post.h"
#include "database.h"

#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace database
{
    Post Post::fromJSON(const std::string &str)
    {
        int start = str.find("_id");
        int end = str.find(",",start);

        std::string s1 = str.substr(0,start-1);
        std::string s2 = str.substr(end+1);

        // std::cout << s1 << s2 << std::endl;
        // std::cout << "from json:" << str << std::endl;
        Post post;

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(s1+s2);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        post.id() = object->getValue<long>("id");
        post.user_id() = object->getValue<long>("user_id");
        post.text() = object->getValue<std::string>("text");
        post.date() = object->getValue<std::string>("date");

        return post;
    }

    long Post::get_id() const
    {
        return _id;
    }

    long Post::get_user_id() const
    {
        return _user_id;
    }

    const std::string &Post::get_text() const
    {
        return _text;
    }

    const std::string &Post::get_date() const
    {
        return _date;
    }
    
    long &Post::id()
    {
        return _id;
    }
    long &Post::user_id()
    {
        return _user_id;
    }

    std::string &Post::text()
    {
        return _text;
    }

    std::string &Post::date()
    {
        return _date;
    }

    std::optional<Post> Post::read_by_id(long id)
    {
        std::optional<Post> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("mongo_db.post",params);

        if(!results.empty())
            result = fromJSON(results[0]);
        
        return result;
    }

    std::vector<Post> Post::read_by_user_id(long user_id)
    {
        std::vector<Post> result;
        std::map<std::string,long> params;
        params["user_id"] = user_id;

        std::vector<std::string> results = database::Database::get().get_from_mongo("mongo_db.post",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));
        

        return result;
    }

    void Post::add()
    {
        database::Database::get().send_to_mongo("mongo_db.post",toJSON());
    }

    void Post::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;       
        database::Database::get().update_mongo("mongo_db.post",params,toJSON());
    }
    Poco::JSON::Object::Ptr Post::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("user_id", _user_id);
        root->set("text", _text);
        root->set("date", _date);

        Poco::JSON::Object::Ptr address = new Poco::JSON::Object();

        return root;
    }
}