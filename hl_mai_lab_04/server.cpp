#include "json_type_handlers.h"
#include "userData.h"

#include <iostream>
#include <string>
#include <vector>

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/Exception.h>
#include <Poco/ThreadPool.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Logger.h>
#include <Poco/URI.h>


using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::MessageHeader;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;


class MessengerHandler : public HTTPRequestHandler
{
public:
    MessengerHandler() : _session(nullptr) {
        _logger.setLevel("information");
    }
    
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override
    {
        try
        {
            // Получение сессии с базой данных
            if (!_session)
            {
                Poco::Data::PostgreSQL::Connector::registerConnector();
                _session = new Session("PostgreSQL", "host=db dbname=user_db user=postgres password=password");
            }

            // Разбор URI запроса
            std::string uri = request.getURI();
            std::string path = uri;
            std::string query;

            // Разделение пути и параметров запроса
            std::size_t pos = uri.find('?');
            if (pos != std::string::npos)
            {
                path = uri.substr(0, pos);
                query = uri.substr(pos + 1);
            }

            Poco::URI requestUri(path);
            std::vector<std::string> parts;
            requestUri.getPathSegments(parts);

            _logger.information("Request URI: " + uri);
            _logger.information("Request path: " + path);
            _logger.information("Request query: " + query);
            _logger.information("Parts: ");
            for (auto part : parts)
            {
                _logger.information(part);
            }
            
            
            if (parts.size() >= 1)
            {
                std::string resource = parts[0];
                
                if (resource == "users")
                {
                    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
                    {                        
                        if (parts.size() == 1)
                        {
                            // Получение списка всех пользователей                            
                            try {
                                Statement select(*_session);
                                select << "SELECT id, login, first_name, last_name, email, title FROM users", now;
                                select.execute();
                                Poco::Data::RecordSet rs(select);
                                Poco::JSON::Array jsonUsers;

                                bool more = rs.moveFirst();
                                while (more) {
                                    Poco::JSON::Object jsonObj;
                                    for (size_t i = 0; i < rs.columnCount(); ++i) {
                                        jsonObj.set(rs.columnName(i), rs[i].convert<std::string>());
                                    }
                                    jsonUsers.add(jsonObj);
                                    more = rs.moveNext();
                                }

                                Poco::JSON::Object result;
                                result.set("users", jsonUsers);

                                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                                response.setContentType("application/json");
                                std::ostream& ostr = response.send();
                                Poco::JSON::Stringifier::stringify(result, ostr);
                            } catch (const Poco::Exception& exc) {
                                _logger.information(exc.displayText());
                                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                                response.send();
                            }

                        }
                        else if (parts.size() == 2 && parts[1] == "login")
                        {
                            // Поиск пользователя по логину
                            try
                            {
                                std::string login = request.get("login");
                                
                                Poco::JSON::Object user;
                                Statement select(*_session);
                                select << "SELECT id, login, first_name, last_name, email, title FROM users WHERE login = $1",
                                    into(user),
                                    use(login),
                                    now;
                                
                                if (user.size() > 0)
                                {
                                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                                    response.setContentType("application/json");
                                    std::ostream& ostr = response.send();
                                    Poco::JSON::Stringifier::stringify(user, ostr);
                                }
                                else
                                {
                                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                                    response.send();
                                }
                            }
                            catch (Poco::Exception& exc)
                            {
                                std::cerr << exc.displayText() << std::endl;
                                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                                response.send();
                            }
                        }
                        else if (parts.size() == 2)
                        {
                            // Получение пользователя по id
                            try {
                                int user_id = std::stoi(parts[1]);
                                
                                UserData userData;
                                Poco::Data::Statement select(*_session);
                                select << "SELECT id, login, first_name, last_name, email, title FROM users WHERE id = $1", 
                                    use(user_id), 
                                    into(userData.id), 
                                    into(userData.login), 
                                    into(userData.first_name), 
                                    into(userData.last_name), 
                                    into(userData.email), 
                                    into(userData.title), 
                                    now;

                                // Проверяем, были ли извлечены какие-либо данные
                                if (select.done() && userData.id != 0) {
                                    Poco::JSON::Object::Ptr user = new Poco::JSON::Object;
                                    user->set("id", userData.id);
                                    user->set("login", userData.login);
                                    user->set("first_name", userData.first_name);
                                    user->set("last_name", userData.last_name);
                                    user->set("email", userData.email);
                                    user->set("title", userData.title);

                                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                                    response.setContentType("application/json");
                                    std::ostream& ostr = response.send();
                                    Poco::JSON::Stringifier::stringify(user, ostr);
                                } else {
                                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                                    response.send();
                                }
                            } catch (const Poco::Exception& exc) {
                                std::cerr << exc.displayText() << std::endl;
                                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                                response.send();
                            }
                        }
                        else if (parts.size() == 3 && parts[1] == "search")
                        {
                            // Поиск пользователей по логину, имени и фамилии
                            try
                            {
                                std::string query = parts[3];
                                
                                Poco::JSON::Array users;
                                std::string pattern = "%" + query + "%";
                                Statement select(*_session);
                                select << "SELECT id, login, first_name, last_name, email, title FROM users WHERE login LIKE $1 OR first_name LIKE $2 OR last_name LIKE $3 OR email LIKE $4 OR title LIKE $5",
                                    into(users),
                                    use(pattern),
                                    use(pattern),
                                    use(pattern),
                                    now;
                                
                                Poco::JSON::Object result;
                                result.set("users", users);
                                
                                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                                response.setContentType("application/json");
                                std::ostream& ostr = response.send();
                                Poco::JSON::Stringifier::stringify(result, ostr);
                            }
                            catch (Poco::Exception& exc)
                            {
                                std::cerr << exc.displayText() << std::endl;
                                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                                response.send();
                            }
                        }
                        else
                        {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                            response.send();
                        }
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
                    {
                        // Создание нового пользователя
                        try
                        {
                            Poco::JSON::Parser parser;
                            Poco::Dynamic::Var result = parser.parse(request.stream());
                            Poco::JSON::Object::Ptr user = result.extract<Poco::JSON::Object::Ptr>();
                            
                            std::string login = user->getValue<std::string>("login");
                            std::string password = user->getValue<std::string>("password");
                            std::string first_name = user->getValue<std::string>("first_name");
                            std::string last_name = user->getValue<std::string>("last_name");
                            std::string email = user->getValue<std::string>("email");
                            std::string title = user->getValue<std::string>("title");

                            _logger.information(login);
                            _logger.information(password);
                            _logger.information(first_name);
                            _logger.information(last_name);
                            _logger.information(email);
                            _logger.information(title);
                            
                            std::string password_hash = hashPassword(password);
                            
                            Statement insert(*_session);
                            insert << "INSERT INTO users (login, password_hash, first_name, last_name, email, title) VALUES ($1, $2, $3, $4, $5, $6)",
                                use(login),
                                use(password_hash),
                                use(first_name),
                                use(last_name),
                                use(email),
                                use(title);
                            insert.execute();
                            
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
                            response.send();
                        }
                        catch (Poco::Exception& exc)
                        {
                            std::cerr << exc.displayText() << std::endl;
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                            response.send();
                        }
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
                    {
                        // Обновление данных пользователя
                        try
                        {
                            int user_id = std::stoi(parts[1]);

                            Poco::JSON::Parser parser;
                            Poco::Dynamic::Var result = parser.parse(request.stream());
                            Poco::JSON::Object::Ptr user = result.extract<Poco::JSON::Object::Ptr>();
                            
                            std::string login = user->getValue<std::string>("login");
                            std::string first_name = user->getValue<std::string>("first_name");
                            std::string last_name = user->getValue<std::string>("last_name");
                            std::string email = user->getValue<std::string>("email");
                            std::string title = user->getValue<std::string>("title");
                            
                            Statement update(*_session);
                            update << "UPDATE users SET login = $1, first_name = $2, last_name = $3, email = $4, title = $5  WHERE id = $6",
                                use(login),
                                use(first_name),
                                use(last_name),
                                use(email),
                                use(title),
                                use(user_id),
                                now;
                            
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.send();
                        }
                        catch (Poco::Exception& exc)
                        {
                            std::cerr << exc.displayText() << std::endl;
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                            response.send();
                        }
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
                    {
                        // Удаление пользователя
                        try
                        {
                            int user_id = std::stoi(parts[2]);
                            
                            Statement delete_(*_session);
                            delete_ << "DELETE FROM users WHERE id = ?",
                                use(user_id),
                                now;
                            
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.send();
                        }
                        catch (Poco::Exception& exc)
                        {
                            std::cerr << exc.displayText() << std::endl;
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                            response.send();
                        }
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                        response.send();
                    }
                }
                else
                {
                    // Обработка некорректного URI
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                    response.send();
                    return;
                }
            }
            else
            {
                // Обработка некорректного URI
                _logger.error("Bad Request: Invalid URI format");
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                
                Poco::JSON::Object errorResponse;
                errorResponse.set("error", "Invalid URI format");
                
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(errorResponse, ostr);
                return;
            }
        }
        catch (Poco::Exception& exc)
        {
            _logger.error("Exception: " + exc.displayText());
            std::cerr << exc.displayText() << std::endl;
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.send();
        }
    }
    
private:
    Session* _session;
    Poco::Logger& _logger = Poco::Logger::get("MessengerHandler");

    std::string hashPassword(const std::string& password)
    {
        Poco::Crypto::DigestEngine digestEngine("SHA256");
        digestEngine.update(password);
        return Poco::Crypto::DigestEngine::digestToHex(digestEngine.digest());
    }

    void sendErrorResponse(HTTPServerResponse& response, const std::string& message, Poco::Net::HTTPResponse::HTTPStatus status)
    {
        _logger.error("Error: " + message);
        response.setStatus(status);
        response.setContentType("application/json");
        
        Poco::JSON::Object errorResponse;
        errorResponse.set("error", message);
        
        std::ostream& ostr = response.send();
        Poco::JSON::Stringifier::stringify(errorResponse, ostr);
    }
};


class MessengerHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    MessengerHandlerFactory(const std::string& url) : _url(url) {}
    
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&) override
    {
        return new MessengerHandler;
    }
    
private:
    std::string _url;
};


class MessengerServer : public ServerApplication
{
public:
    MessengerServer() : _helpRequested(false) {}
    
    ~MessengerServer() {}
    
protected:
    void initialize(Application& self) override
    {
        loadConfiguration();
        ServerApplication::initialize(self);
    }
    
    void defineOptions(OptionSet& options) override
    {
        ServerApplication::defineOptions(options);

        options.addOption(
            Option("help", "h", "Display help information on command line arguments.")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<MessengerServer>(this, &MessengerServer::handleHelp)));
    }
    
    void handleHelp(const std::string& name, const std::string& value)
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A messenger server application.");
        helpFormatter.format(std::cout);
        stopOptionsProcessing();
        _helpRequested = true;
    }
    
    int main(const std::vector<std::string>& args) override
    {
        if (!_helpRequested)
        {
            unsigned short port = (unsigned short)config().getInt("MessengerServer.port", 8080);

            ServerSocket svs(port);
            HTTPServer srv(new MessengerHandlerFactory("/"), svs, new HTTPServerParams);

            srv.start();
            waitForTerminationRequest();
            srv.stop();
        }
        return Application::EXIT_OK;
    }
    
private:
    bool _helpRequested;
};


int main(int argc, char** argv)
{
    MessengerServer app;
    return app.run(argc, argv);
}