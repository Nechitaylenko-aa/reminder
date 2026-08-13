cmake_minimum_required(VERSION 3.19)
project(reminder LANGUAGES CXX)

include(FetchContent)
# databased: optional dependency
FetchContent_Declare(
        databased
        GIT_REPOSITORY git@192.168.1.102:artem/databases.git
        GIT_TAG master
)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_GUI "Build GUI application" ON)
option(USE_DATABASE "Enable database dependency" ON)

# If running tests, disable database by default to allow building on CI without private repo
if(BUILD_TESTS)
    # Only change USE_DATABASE if user didn't explicitly pass it
    if(NOT DEFINED CMAKE_CACHE_VARIABLE_SET_USE_DATABASE)
        set(USE_DATABASE OFF CACHE BOOL "Enable database dependency" FORCE)
    endif()
endif()

# Catch2 for tests (only fetched when BUILD_TESTS)
if(BUILD_TESTS)
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
  )
  FetchContent_MakeAvailable(catch2)
endif()

# Find Qt only if building GUI
if(BUILD_GUI)
  find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    add_compile_definitions(
            FreeBSD
    )
endif()

include_directories(
        ${CMAKE_SOURCE_DIR}
)

# --- backend core sources (compiled without Qt) ---
file(GLOB_RECURSE BACKEND_CORE_SOURCES
    backend/sources/src/*.cpp
    backend/db-models/*.cpp
    backend/db-models/units/*.cpp
)

# --- original backend and gui globs (used for full app) ---
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

# --- reminder_core library (optional, for tests) ---
if(BUILD_TESTS)
    add_library(reminder_core STATIC ${BACKEND_CORE_SOURCES})
    target_include_directories(reminder_core PUBLIC
        ${CMAKE_SOURCE_DIR}/backend/sources/include
        ${CMAKE_SOURCE_DIR}/backend/db-models
        ${CMAKE_SOURCE_DIR}
    )
    if(USE_DATABASE)
        FetchContent_MakeAvailable(databased)
        target_link_libraries(reminder_core PRIVATE database)
    endif()
endif()

# --- GUI executable (only if BUILD_GUI) ---
if(BUILD_GUI)
  qt_add_executable(reminder
      WIN32 MACOSX_BUNDLE
      main.cpp
      ${BACKEND}
      ${GUI}
  )

  if(USE_DATABASE)
    target_include_directories(reminder PRIVATE
            ${database_SOURCE_DIR}/include
            )
  endif()

  target_link_libraries(reminder
      PRIVATE
          Qt::Core
          Qt::Widgets
  )

  if(USE_DATABASE)
      target_link_libraries(reminder PRIVATE database)
  endif()

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
endif()

# --- add tests subdirectory if enabled ---
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
