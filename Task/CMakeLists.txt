add_library(Task INTERFACE)

target_include_directories(Task INTERFACE ./include)

target_sources(Task
        INTERFACE
        Task.cpp
)

