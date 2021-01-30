cmake_minimum_required(VERSION 3.1)
project(YDNES)

# Add sources
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Specify include Directory
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(YDNES ${SOURCES})