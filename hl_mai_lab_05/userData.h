#include <iostream>

struct UserData {
    long id;
    std::string login;
    std::string password;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::string title;

    UserData() : id(0) {}

    void clear() {
        id = 0;
        login.clear();
        password.clear();
        first_name.clear();
        last_name.clear();
        email.clear();
        title.clear();
    }
};