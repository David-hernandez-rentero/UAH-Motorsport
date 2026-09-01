cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

# Set the project name
set(CMAKE_PROJECT_NAME G474Re-BMS)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE BmsFiles
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/bms/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/bms/*.h"
)

# Set all warnings
set_source_files_properties(
        ${BmsFiles}
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wpedantic;-Werror"
)

# Deterministic builds
target_compile_options(
        ${CMAKE_PROJECT_NAME} PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
)

# UBSan for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fsanitize=undefined)
endif()

# Git hash
execute_process(
        COMMAND git ls-files -s .
        COMMAND git hash-object --stdin
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE GIT_HASH=${GIT_HASH})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
        ${BmsFiles}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx

        # Add user defined libraries
)
