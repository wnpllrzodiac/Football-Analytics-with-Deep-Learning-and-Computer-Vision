cmake_minimum_required(VERSION 3.15)
project(FootballAnalytics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出编译信息
message(STATUS "Building Football Analytics C++ Project")

# ==================== 依赖库配置 ====================

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# FFmpeg
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libavutil
        libswscale
    )
    message(STATUS "FFmpeg libraries found")
endif()

# ONNX Runtime
# 请根据实际安装路径调整
set(ONNXRUNTIME_DIR "C:/onnxruntime" CACHE PATH "ONNX Runtime installation directory")
if(EXISTS ${ONNXRUNTIME_DIR})
    include_directories(${ONNXRUNTIME_DIR}/include)
    link_directories(${ONNXRUNTIME_DIR}/lib)
    message(STATUS "ONNX Runtime found at: ${ONNXRUNTIME_DIR}")
else()
    message(WARNING "ONNX Runtime not found. Please set ONNXRUNTIME_DIR")
endif()

# JSON library (nlohmann/json - header-only)
find_package(nlohmann_json CONFIG)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, will use submodule or download")
    # 可以通过 vcpkg install nlohmann-json 安装
endif()

# ==================== 项目配置 ====================

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 源文件
set(SOURCES
    src/main.cpp
    src/VideoReader.cpp
    src/YOLODetector.cpp
    src/TeamPredictor.cpp
    src/CoordinateTransform.cpp
    src/ApiClient.cpp
)

# 可执行文件
add_executable(football_analytics ${SOURCES})

# ==================== 链接库 ====================

target_link_libraries(football_analytics
    ${OpenCV_LIBS}
    onnxruntime
)

# FFmpeg链接
if(PkgConfig_FOUND)
    target_link_libraries(football_analytics PkgConfig::LIBAV)
endif()

# JSON链接（如果找到）
if(nlohmann_json_FOUND)
    target_link_libraries(football_analytics nlohmann_json::nlohmann_json)
endif()

# Windows特定设置
if(WIN32)
    target_link_libraries(football_analytics ws2_32)
endif()

# ==================== 编译选项 ====================

if(MSVC)
    target_compile_options(football_analytics PRIVATE /W4)
else()
    target_compile_options(football_analytics PRIVATE -Wall -Wextra)
endif()

# ==================== 安装配置 ====================

install(TARGETS football_analytics DESTINATION bin)
install(DIRECTORY models/ DESTINATION models)
install(DIRECTORY config/ DESTINATION config)
install(DIRECTORY resources/ DESTINATION resources)

message(STATUS "Configuration complete. Run 'cmake --build .' to build the project.")
