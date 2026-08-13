cmake_minimum_required(VERSION 3.19)
project(reminder LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        databased
        GIT_REPOSITORY git@192.168.1.102:artem/databases.git
        GIT_TAG master
)

FetchContent_MakeAvailable(databased)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    add_compile_definitions(
            FreeBSD
    )
endif()

include_directories(
        ${CMAKE_SOURCE_DIR}
)

file(GLOB_RECURSE BACKEND
    backend/*.cpp
    backend/*.h
    )

file(GLOB_RECURSE GUI
    gui/*.cpp
    gui/*.h
    gui/*.ui
    gui/*.qrc
    )

qt_add_executable(reminder
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${BACKEND}
    ${GUI}
)

target_include_directories(reminder PRIVATE
        ${database_SOURCE_DIR}/include
        )

target_link_libraries(reminder
    PRIVATE
        Qt::Core
        Qt::Widgets
        database
)

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

include(GNUInstallDirs)

install(TARGETS reminder
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET reminder
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
