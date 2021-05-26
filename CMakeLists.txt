cmake_minimum_required(VERSION 3.0.0)
project(onekey-proxy VERSION 0.2)

include(CTest)
enable_testing()

message(STATUS "${PROJECT_NAME} started building on ${CMAKE_SYSTEM_NAME}!")

set(include_dirs "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/json")
message(STATUS "Include dirs: ${include_dirs}")
include_directories(${include_dirs})

# Src
file(GLOB_RECURSE source_files
        "${PROJECT_SOURCE_DIR}/src/json/*.cpp" "${PROJECT_SOURCE_DIR}/src/json/*.h")
message(STATUS "Source files: ${source_files}")

# Linking
# link_libraries(-lpthread glog::glog ${OpenCV_LIBS})

add_executable(onekey-proxy ${PROJECT_SOURCE_DIR}/src/main.cpp ${source_files})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
