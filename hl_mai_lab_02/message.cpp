#include "message.h"
#include "database.h"

#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace database
{
    Message Message::fromJSON(const std::string &str)
    {
        int start = str.find("_id");
        int end = str.find(",",start);

        std::string s1 = str.substr(0,start-1);
        std::string s2 = str.substr(end+1);

        // std::cout << s1 << s2 << std::endl;
        // std::cout << "from json:" << str << std::endl;
        Message mes;

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(s1+s2);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        mes.id() = object->getValue<long>("id");
        mes.sender_id() = object->getValue<long>("sender_id");
        mes.recipient_id() = object->getValue<long>("recipient_id");
        mes.text() = object->getValue<std::string>("text");
        mes.date() = object->getValue<std::string>("date");

        return mes;
    }

    long Message::get_id() const
    {
        return _id;
    }

    long Message::get_sender_id() const
    {
        return _sender_id;
    }

    long Message::get_recipient_id() const
    {
        return _recipient_id;
    }

    const std::string &Message::get_text() const
    {
        return _text;
    }

    const std::string &Message::get_date() const
    {
        return _date;
    }
    
    long &Message::id()
    {
        return _id;
    }
    long &Message::sender_id()
    {
        return _sender_id;
    }
    long &Message::recipient_id()
    {
        return _recipient_id;
    }

    std::string &Message::text()
    {
        return _text;
    }

    std::string &Message::date()
    {
        return _date;
    }

    std::optional<Message> Message::read_by_id(long id)
    {
        std::optional<Message> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("mongo_db.message",params);

        if(!results.empty())
            result = fromJSON(results[0]);
        
        return result;
    }

    std::vector<Message> Message::read_by_sender_id(long sender_id)
    {
        std::vector<Message> result;
        std::map<std::string,long> params;
        params["sender_id"] = sender_id;

        std::vector<std::string> results = database::Database::get().get_from_mongo("mongo_db.message",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));
        

        return result;
    }

    std::vector<Message> Message::read_by_recipient_id(long recipient_id)
    {
        std::vector<Message> result;
        std::map<std::string,long> params;
        params["recipient_id"] = recipient_id;

        std::vector<std::string> results = database::Database::get().get_from_mongo("mongo_db.message",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));
        

        return result;
    }

    void Message::add()
    {
        database::Database::get().send_to_mongo("mongo_db.message",toJSON());
    }

    void Message::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;       
        database::Database::get().update_mongo("mongo_db.message",params,toJSON());
    }
    Poco::JSON::Object::Ptr Message::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("sender_id", _sender_id);
        root->set("recipient_id", _recipient_id);
        root->set("text", _text);
        root->set("date", _date);

        Poco::JSON::Object::Ptr address = new Poco::JSON::Object();

        return root;
    }
}