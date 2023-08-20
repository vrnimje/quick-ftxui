cmake_minimum_required(VERSION 3.14)

add_executable(button buttons.cpp)
target_link_libraries(button
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

target_include_directories(button
    PRIVATE ../include
)