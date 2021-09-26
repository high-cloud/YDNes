cmake_minimum_required(VERSION 3.1)
project(YDNES)

# Add sources
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/include/*.h"
)

# Specify include Directory
include_directories("${PROJECT_SOURCE_DIR}/include")

# Find SFML
set(SFML_DIR "E:/code/SFML-2.5.1/lib/cmake/SFML")
set(SFML_STATIC_LIBRARIES TRUE)
find_package( SFML 2.5 COMPONENTS graphics window REQUIRED)

set(SFML_STATIC_LIBRARIES TRUE)
if(SFML_FOUND)
        include_directories("E:/code/SFML-2.5.1/include")
else()
        set(SFML_ROOT "E:/code/SFML-2.5.1")
        message("\nSFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
        message("Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

add_executable(YDNES ${SOURCES})
target_link_libraries(YDNES sfml-graphics sfml-window)

target_link_libraries(YDNES)

# use to set compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)