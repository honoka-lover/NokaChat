cmake_minimum_required(VERSION 3.24)
project(Vanilla)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "E:/qt6/Tools/CMake_64/bin")
set(CMAKE_PREFIX_PATH "E:/qt6/6.5.2/mingw_64/lib/cmake")

#set(CMAKE_PREFIX_PATH "E:/qt5/5.12.8/mingw73_64/lib/cmake")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

find_package(OpenSSL REQUIRED)

include_directories( .
    ./Component
    E:/qt/Tools/OpenSSLv3/Win_x64/include/openssl
)

file(GLOB AllFile "./*.h" "./*.cpp" "./Component/*.h" "./Component/*.cpp" "./*.ui" "./Component/*.ui")

add_executable(Vanilla
        ${AllFile}
        resource.qrc)
target_link_libraries(Vanilla
        PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        OpenSSL::SSL 
        OpenSSL::Crypto
)




if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
   set(DEBUG_SUFFIX)
   if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
       set(DEBUG_SUFFIX "d")
   endif ()
   set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
   if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
       set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
       if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
           set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
       endif ()
   endif ()
   if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
       add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E make_directory
               "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
       add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E copy
               "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
               "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
   endif ()
   foreach (QT_LIB Core Gui Widgets Network)
       add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E copy
               "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
               "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
   endforeach (QT_LIB)
endif ()
