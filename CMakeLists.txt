cmake_minimum_required(VERSION 3.10)
project(c_chess C)

set(CMAKE_C_STANDARD 99)

# Print out more information during cmake configuration
message(STATUS "Configuring chess project with API support...")

# Option to specify custom curl directory
option(CURL_DIR "Path to libcurl installation directory" "")

# Add common installation paths for libcurl to search path
list(APPEND CMAKE_PREFIX_PATH 
    "C:/Program Files/CURL" 
    "C:/curl"
    "C:/vcpkg/installed/x64-windows"
    "C:/vcpkg/installed/x86-windows"
    "$ENV{VCPKG_ROOT}/installed/x64-windows"
    "$ENV{VCPKG_ROOT}/installed/x86-windows"
)

# If user provided a custom curl directory, add it to the search path
if(CURL_DIR)
    message(STATUS "Looking for libcurl in custom directory: ${CURL_DIR}")
    list(APPEND CMAKE_PREFIX_PATH ${CURL_DIR})
endif()

# Try to find curl
set(CURL_NO_CURL_CMAKE "ON")
find_package(CURL QUIET)

# If not found on the first try, look for curl-config
if(NOT CURL_FOUND)
    message(STATUS "libcurl not found with basic find_package, trying alternative approaches...")
    
    # Try pkg-config approach
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CURL libcurl)
        if(CURL_FOUND)
            message(STATUS "Found libcurl using pkg-config")
        endif()
    endif()
endif()

# Include all source files in the project
add_executable(c_chess 
    main.c 
    board.c 
    moves.c
    api.c
)

# If CURL is found, use it; otherwise, define a preprocessor macro to disable API features
if(CURL_FOUND)
    message(STATUS "Found CURL: ${CURL_LIBRARIES}")
    message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
    
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(c_chess ${CURL_LIBRARIES})
    
    # Add a compile definition to indicate CURL is available
    target_compile_definitions(c_chess PRIVATE CURL_SUPPORT=1)
    
    message(STATUS "Building with API support enabled")
else()
    message(STATUS "CURL not found, building without API support")
    message(STATUS "To enable API support, install libcurl and its development files or specify CURL_DIR")
    message(STATUS "For Windows:")
    message(STATUS "  - Install libcurl via vcpkg: vcpkg install curl")
    message(STATUS "  - Then use: cmake -DCURL_DIR=C:/path/to/curl ..")
    message(STATUS "For Linux: sudo apt-get install libcurl4-openssl-dev")
    message(STATUS "For macOS: brew install curl")
    
    target_compile_definitions(c_chess PRIVATE NO_CURL_SUPPORT)
endif()

# Print a summary of the configuration
message(STATUS "Configuration summary:")
message(STATUS "  API support: ${CURL_FOUND}")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
