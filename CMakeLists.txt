cmake_minimum_required(VERSION 3.16)
project(BookManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Svg)

add_executable(BookManager
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    pages.h pages.cpp parametrespage.ui
    widgets.h widgets.cpp
    theme.h theme.cpp
    data.h data.cpp
    dialogs.h dialogs.cpp
    resources.qrc
)
target_link_libraries(BookManager PRIVATE Qt6::Widgets Qt6::Sql Qt6::Svg)
