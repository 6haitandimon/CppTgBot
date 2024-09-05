cmake_minimum_required(VERSION 3.28)
project(RobboTGBot)

set(CMAKE_CXX_STANDARD 17)

set(TGBOTCPP_INCLUDE_DIR "/usr/local/include")
set(TGBOTCPP_LIB_DIR "/usr/local/lib")
set(SQLITECPP_INCLUDE_DIR "/usr/local/include")
set(SQLITECPP_LIB_DIR "/usr/local/lib")

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)
find_package(hiredis REQUIRED)
find_package(redis++ REQUIRED)
find_package(unofficial-mysql-connector-cpp REQUIRED)

include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

include_directories(${TGBOTCPP_INCLUDE_DIR} ${SQLITECPP_INCLUDE_DIR} include)
link_directories(${TGBOTCPP_LIB_DIR} ${SQLITECPP_LIB_DIR})

add_subdirectory(StateMachine)
add_subdirectory(User)
add_subdirectory(Task)

file(GLOB SOURCES "src/*.cpp")

add_executable(RobboTGBot ${SOURCES})

target_link_libraries(RobboTGBot
        tgbot
        hiredis::hiredis
        redis++::redis++
        unofficial::mysql-connector-cpp::connector
        sqlite3
        SQLiteCpp
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
        StateMachine
        User
        Task
        "-framework CoreFoundation"
        "-framework Security"
        "-lresolv"
)