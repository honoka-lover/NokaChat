cmake_minimum_required(VERSION 3.24)
project(Vanilla)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# set(CMAKE_PREFIX_PATH "E:/qt6/6.5.2/mingw_64/lib/cmake")
#set(CMAKE_PREFIX_PATH "F:/QT/6.6.1/mingw_64/lib/cmake")
#set(CMAKE_PREFIX_PATH "F:/QT/6.6.1/msvc2019_64/lib/cmake")
set(CMAKE_PREFIX_PATH "E:/qt6/6.6.3/msvc2019_64/lib/cmake")

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
    file(GLOB_RECURSE ALL_FILES "${DIR}/*.h" "${DIR}/*.cpp" "${DIR}/*.ui" )

    set(${RESULT} ${ALL_FILES} PARENT_SCOPE)
endfunction()

# 调用函数以获取所有文件
get_all_files_in_directory(${SRC_DIR} ALL_SRC_FILES)

get_all_files_in_directory(${INCLUDE_DIR} ALL_HEAD_FILES)
# 创建一个新的列表来存储过滤后的文件
set(FILTERED_SRC_FILES)
set(FILTERED_HEAD_FILES)
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
#遍历include中头文件
foreach(FILE ${ALL_HEAD_FILES})
    # 获取文件名
    get_filename_component(FILENAME ${FILE} NAME)
    list(APPEND FILTERED_HEAD_FILES ${FILE})
endforeach()

# 打印过滤后的文件列表（可选，用于调试）
message(STATUS "Filtered source files: ${FILTERED_SRC_FILES}")

include_directories(src include)
file(GLOB LibFile "lib/*.lib")
file(GLOB DllFile "lib/*.dll")
add_executable(Vanilla
        ${FILTERED_SRC_FILES}
        ${FILTERED_HEAD_FILES}
        source/resource.qrc
        source/myicon.rc
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
#复制dll和配置文件到执行目录
configure_file(source/setting.ini ${CMAKE_CURRENT_BINARY_DIR}/setting.ini COPYONLY)
file(COPY ${DllFile} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# 安装文件
install(TARGETS Vanilla DESTINATION bin)
install(FILES ${LibFile} ${DllFile} DESTINATION bin)
#install(FILES ${FILTERED_HEAD_FILES} DESTINATION include)
# 打包
set(CPACK_PACKAGE_NAME "Vanilla")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "ZIP")
include(CPack)

