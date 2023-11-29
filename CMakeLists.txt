cmake_minimum_required(VERSION 3.24)
project(Vanilla)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "E:/qt6/Tools/CMake_64/bin")
set(CMAKE_PREFIX_PATH "E:/qt6/6.5.2/mingw_64/lib/cmake")

set(INC_DIR E:/qt6/Tools/OpenSSLv3/Win_x64/include)
set(LINK_DIR E:/qt6/Tools/OpenSSLv3/Win_x64/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(libcrypto libssl)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

find_package(OpenSSL REQUIRED)

link_libraries("E:/qt6/Tools/OpenSSLv3/Win_x64/lib/libssl.lib")

include_directories( .
    ./Component
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
        Qt6::Network
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
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
