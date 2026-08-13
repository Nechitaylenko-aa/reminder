cmake_minimum_required(VERSION 3.19)
project(reminder LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        databased
        GIT_REPOSITORY git@github.com:Nechitaylenko-aa/lib-database.git
        GIT_TAG dev
)

FetchContent_MakeAvailable(databased)

# Options for embedding/running tests in the application
option(BUILD_TESTS_IN_APP "Compile tests into application binary" ON)
option(RUN_TESTS_AT_STARTUP "Run embedded tests at application startup before launching GUI" ON)

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

# Include database headers
target_include_directories(reminder PRIVATE
        ${database_SOURCE_DIR}/include
        )

# Link libraries
target_link_libraries(reminder
        PRIVATE
        Qt::Core
        Qt::Widgets
        database
        )

# If requested, embed tests into application
if(BUILD_TESTS_IN_APP)
    include(FetchContent)
    FetchContent_Declare(
      catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(catch2)

    # expose macro into compilation units
    if(RUN_TESTS_AT_STARTUP)
        target_compile_definitions(reminder PRIVATE ENABLE_TESTS_IN_APP ENABLE_TESTS_AT_STARTUP)
    else()
        target_compile_definitions(reminder PRIVATE ENABLE_TESTS_IN_APP)
    endif()

    target_link_libraries(reminder PRIVATE Catch2::Catch2)

    # Add test sources into application binary
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
    target_sources(reminder PRIVATE ${TEST_SOURCES})
endif()

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
