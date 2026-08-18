cmake_minimum_required(VERSION 3.16)
project(TableViewApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    main.cpp
    src/mainwindow.cpp
    src/tablemodel.cpp
    src/importer.cpp
    src/databaseworker.cpp
    src/importworker.cpp
    src/progressdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/tablemodel.h
    src/importer.h
    src/databaseworker.h
    src/importworker.h
    src/progressdialog.h
)

set(FORMS
    src/mainwindow.ui
)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
)
