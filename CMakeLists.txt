cmake_minimum_required(VERSION 3.24)
project(Vanilla)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    # Visual Studio specific settings
    set(CMAKE_PREFIX_PATH "E:/qt6/6.6.3/msvc2019_64/lib/cmake")
else()
    # MinGW specific settings
    set(CMAKE_PREFIX_PATH "E:/qt6/6.6.3/mingw_64/lib/cmake")
endif()

#message("CMAKE_PREFIX_PATH: "${CMAKE_PREFIX_PATH})

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        Multimedia
        OpenGLWidgets
        REQUIRED)

# 设置源文件目录
set(SRC_DIR "src")

set(INCLUDE_DIR "include")
# 定义一个函数来递归获取目录中的所有文件
function(get_all_files_in_directory DIR RESULT)
    file(GLOB_RECURSE ALL_FILES "${DIR}/*.h" "${DIR}/*.cpp" "${DIR}/*.ui" "${DIR}/*.hpp")

    set(${RESULT} ${ALL_FILES} PARENT_SCOPE)
endfunction()

# 调用函数以获取所有文件
get_all_files_in_directory(${SRC_DIR} ALL_SRC_FILES)
# 创建一个新的列表来存储过滤后的文件
set(FILTERED_SRC_FILES)
# 遍历所有文件
foreach(FILE ${ALL_SRC_FILES})
    # 获取文件名
    get_filename_component(FILENAME ${FILE} NAME)

    # 检查文件名是否以 ui_ 开头
    if(NOT FILENAME MATCHES "^ui_")
        # 如果不是，则将其添加到过滤后的文件列表中
        list(APPEND FILTERED_SRC_FILES ${FILE})
    endif()
endforeach()

# 打印过滤后的文件列表（可选，用于调试）
# message(STATUS "Filtered source files: ${FILTERED_SRC_FILES}")

include_directories(src include include/freetype)
if (MSVC)
    file(GLOB LibFile "lib/*.lib" "lib/msvc/*.lib" "lib/*.dll.a" "lib/mingw/*.dll.a")
    file(GLOB DllFile "dll/*.dll" "dll/msvc/*.dll")
else ()
    file(GLOB LibFile "lib/*.lib" "lib/mingw/*.lib" "lib/*.dll.a" "lib/mingw/*.dll.a")
    file(GLOB DllFile "dll/*.dll" "dll/mingw/*.dll")
endif ()

add_executable(Vanilla
        ${FILTERED_SRC_FILES}
        resources/resource.qrc
        resources/myicon.rc
)
target_link_libraries(Vanilla
        PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Multimedia
        Qt::OpenGLWidgets
        ${LibFile}
)
set(NAME "Vanilla")
#设置输出目录
if(WIN32)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
        set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")
endif(WIN32)

#复制dll和配置文件到执行目录
configure_file(resources/setting.ini ${CMAKE_SOURCE_DIR}/bin/setting.ini COPYONLY)
file(COPY ${DllFile} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
# file(GLOB SourceFile "source/model")
# file(COPY ${SourceFile} DESTINATION ${CMAKE_SOURCE_DIR}/bin/source)

# 安装文件
install(TARGETS Vanilla DESTINATION bin)
install(FILES ${LibFile} ${DllFile} DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/bin/setting.ini DESTINATION bin)
set(SOURCE_DIR  "resources/model")
install(DIRECTORY  ${SOURCE_DIR} DESTINATION resources)
#install(FILES ${FILTERED_HEAD_FILES} DESTINATION include)
# 打包
set(CPACK_PACKAGE_NAME "Vanilla")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "ZIP")
include(CPack)

