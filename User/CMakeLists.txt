add_library(User INTERFACE)

target_include_directories(User INTERFACE ./include)

target_sources(StateMachine
        INTERFACE
        User.cpp
)

