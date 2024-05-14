cmake_minimum_required(VERSION 3.0)
project(social_network)

include_directories(/usr/local/include)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT MongoDB)
find_package(PostgreSQL REQUIRED)
find_package(Threads)

include_directories(${Poco_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIRS})

link_directories(/usr/local/lib ${Poco_LIBRARY_DIRS} ${PostgreSQL_LIBRARY_DIRS})

set (STD_CXX "c++17")
set (REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=${STD_CXX} ${REDISCPP_FLAGS}")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -std=${STD_CXX} -Wall -DNDEBUG ${REDISCPP_FLAGS}")
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(social_network main.cpp config.cpp database.cpp user.cpp message.cpp post.cpp cache.cpp)
add_executable(social_network_gateway api_gateway/main.cpp config.cpp cache.cpp)

target_include_directories(social_network PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(social_network_gateway PRIVATE "${CMAKE_BINARY_DIR}")

set_target_properties(social_network PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(social_network PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(social_network
    ${Poco_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
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

target_link_libraries(social_network_gateway
    ${Poco_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
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