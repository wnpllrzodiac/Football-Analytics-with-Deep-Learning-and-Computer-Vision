cmake_minimum_required(VERSION 3.15)
project(FootballAnalytics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 检测操作系统
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# 输出编译信息
message(STATUS "Building Football Analytics C++ Project")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ==================== 依赖库配置 ====================

if(WIN32)
    # ==================== Windows 配置 ====================
    message(STATUS "Configuring for Windows...")
    
    # OpenCV (手动配置 - 按用户要求)
    set(OPENCV_DIR "C:/OpenCV4.10.0/opencv/build" CACHE PATH "OpenCV installation directory")
    if(EXISTS ${OPENCV_DIR})
        include_directories(${OPENCV_DIR}/include)
        link_directories(${OPENCV_DIR}/x64/vc16/lib)
        message(STATUS "OpenCV found at: ${OPENCV_DIR}")
    else()
        message(FATAL_ERROR "OpenCV not found. Please set OPENCV_DIR to your OpenCV installation")
    endif()
    
    # FFmpeg
    set(FFMPEG_DIR "D:/sdk/ffmpeg-8.0.1-full_build-shared" CACHE PATH "FFmpeg installation directory")
    if(EXISTS ${FFMPEG_DIR})
        include_directories(${FFMPEG_DIR}/include)
        link_directories(${FFMPEG_DIR}/lib)
        message(STATUS "FFmpeg found at: ${FFMPEG_DIR}")
    else()
        message(WARNING "FFmpeg not found. Please set FFMPEG_DIR")
    endif()
    
    # ONNX Runtime
    set(ONNXRUNTIME_DIR "C:/onnxruntime-win-x64-1.23.2" CACHE PATH "ONNX Runtime installation directory")
    if(EXISTS ${ONNXRUNTIME_DIR})
        include_directories(${ONNXRUNTIME_DIR}/include)
        link_directories(${ONNXRUNTIME_DIR}/lib)
        message(STATUS "ONNX Runtime found at: ${ONNXRUNTIME_DIR}")
    else()
        message(WARNING "ONNX Runtime not found. Please set ONNXRUNTIME_DIR")
    endif()
    
elseif(LINUX)
    # ==================== Linux 配置 ====================
    message(STATUS "Configuring for Linux...")
    
    # OpenCV (使用 find_package)
    find_package(OpenCV 4 REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    
    # FFmpeg (使用 pkg-config)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED
        libavcodec
        libavformat
        libavutil
        libswscale
    )
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIBRARY_DIRS})
    message(STATUS "FFmpeg found: ${FFMPEG_VERSION}")
    
    # ONNX Runtime (GPU 版本)
    set(ONNXRUNTIME_DIR "/home/zodiac/work/tools/onnxruntime-gpu" CACHE PATH "ONNX Runtime installation directory")
    if(EXISTS ${ONNXRUNTIME_DIR})
        include_directories(${ONNXRUNTIME_DIR}/include)
        link_directories(${ONNXRUNTIME_DIR}/lib)
        message(STATUS "ONNX Runtime GPU found at: ${ONNXRUNTIME_DIR}")
    else()
        message(FATAL_ERROR "ONNX Runtime not found. Please set ONNXRUNTIME_DIR or run install_deps_linux.sh")
    endif()
    
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# JSON library (nlohmann/json - header-only)
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

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

if(WIN32)
    # Windows 链接配置
    # OpenCV 库文件 (根据实际版本调整)
    set(OPENCV_LIBS_RELEASE opencv_world4100)
    set(OPENCV_LIBS_DEBUG opencv_world4100d)
    
    target_link_libraries(football_analytics
        optimized ${OPENCV_LIBS_RELEASE}
        debug ${OPENCV_LIBS_DEBUG}
        onnxruntime
        nlohmann_json::nlohmann_json
        avformat
        avcodec
        avutil
        swscale
        ws2_32  # Windows sockets
    )
    
elseif(LINUX)
    # Linux 链接配置
    target_link_libraries(football_analytics
        ${OpenCV_LIBS}
        ${FFMPEG_LIBRARIES}
        onnxruntime
        nlohmann_json::nlohmann_json
        pthread
        dl
    )
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
