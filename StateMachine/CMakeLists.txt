add_library(StateMachine INTERFACE)

target_include_directories(StateMachine INTERFACE ./include)

target_sources(StateMachine
        INTERFACE
        StateMachine.cpp
)

