cmake_minimum_required(VERSION 3.0)
project(social_network)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT MongoDB)
find_package(PostgreSQL REQUIRED)

include_directories(${Poco_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIRS})

link_directories(/usr/local/lib ${Poco_LIBRARY_DIRS} ${PostgreSQL_LIBRARY_DIRS})

add_executable(social_network main.cpp config.cpp database.cpp user.cpp message.cpp post.cpp)

target_link_libraries(social_network
    ${Poco_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
    PocoFoundation
    PocoData
    PocoDataPostgreSQL
    PocoNet
    PocoJSON
    PocoUtil
    PocoCrypto
    PocoJWT
    PocoMongoDB
)