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
    "C:/Program Files (x86)/curl"
    # Add MinGW common paths
    "C:/MinGW/include"
    "C:/MinGW/lib"
    # Add msys2 common paths
    "C:/msys64/mingw64"
    # Add WSL paths if using WSL
    "/usr/include"
    "/usr/lib"
)

# If user provided a custom curl directory, add it to the search path
if(CURL_DIR)
    message(STATUS "Looking for libcurl in custom directory: ${CURL_DIR}")
    list(APPEND CMAKE_PREFIX_PATH ${CURL_DIR})
endif()

# Attempt to find curl libraries - first try the standard approach
find_package(CURL QUIET)

# If not found, try with different configurations
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
    
    # If still not found, try looking for curl.h directly
    if(NOT CURL_FOUND)
        message(STATUS "Searching for curl.h directly...")
        
        # Common include directories to search
        set(CURL_SEARCH_DIRS
            "C:/curl/include"
            "C:/Program Files/curl/include"
            "C:/vcpkg/installed/x64-windows/include"
            "C:/MinGW/include"
            "C:/msys64/mingw64/include"
            "/usr/include"
            "/usr/local/include"
        )
        
        find_path(CURL_INCLUDE_DIR curl/curl.h PATHS ${CURL_SEARCH_DIRS})
        
        if(CURL_INCLUDE_DIR)
            message(STATUS "Found curl.h at: ${CURL_INCLUDE_DIR}")
            
            # Now try to find the library
            find_library(CURL_LIBRARY
                NAMES curl libcurl libcurl_imp
                PATHS
                    "C:/curl/lib"
                    "C:/Program Files/curl/lib"
                    "C:/vcpkg/installed/x64-windows/lib"
                    "C:/MinGW/lib"
                    "C:/msys64/mingw64/lib"
                    "/usr/lib"
                    "/usr/local/lib"
            )
            
            if(CURL_LIBRARY)
                message(STATUS "Found curl library at: ${CURL_LIBRARY}")
                set(CURL_FOUND TRUE)
                set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
                set(CURL_LIBRARIES ${CURL_LIBRARY})
            endif()
        endif()
    endif()
endif()

# Include all source files in the project
add_executable(c_chess 
    main.c 
    board.c 
    moves.c
    api.c
    check.c
    saveload.c
        saveload.c
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
    
    # Copy the DLL to the output directory (on Windows) to ensure it's available at runtime
    if(WIN32)
        # Try to find the actual DLL file
        get_filename_component(CURL_LIB_DIR ${CURL_LIBRARY} DIRECTORY)
        
        # Common DLL names to look for
        set(CURL_DLL_NAMES 
            "libcurl.dll" 
            "curl.dll"
            "libcurl-4.dll"
                "libcurl-x64.dll"
        )
        
        # Look for the DLL in both the lib directory and its parent
        foreach(DLL_NAME ${CURL_DLL_NAMES})
            if(EXISTS "${CURL_LIB_DIR}/${DLL_NAME}")
                message(STATUS "Found curl DLL: ${CURL_LIB_DIR}/${DLL_NAME}")
                add_custom_command(TARGET c_chess POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CURL_LIB_DIR}/${DLL_NAME}"
                    $<TARGET_FILE_DIR:c_chess>)
                break()
            elseif(EXISTS "${CURL_LIB_DIR}/../bin/${DLL_NAME}")
                message(STATUS "Found curl DLL: ${CURL_LIB_DIR}/../bin/${DLL_NAME}")
                add_custom_command(TARGET c_chess POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CURL_LIB_DIR}/../bin/${DLL_NAME}"
                    $<TARGET_FILE_DIR:c_chess>)
                break()
            endif()
        endforeach()
        
        # Look for other required DLLs like zlib or SSL
        set(ADDITIONAL_DLLS
            "zlib1.dll"
            "libssl-1_1.dll"
            "libcrypto-1_1.dll"
            "libeay32.dll"
            "ssleay32.dll"
        )
        
        foreach(DLL_NAME ${ADDITIONAL_DLLS})
            if(EXISTS "${CURL_LIB_DIR}/${DLL_NAME}")
                message(STATUS "Found additional DLL: ${CURL_LIB_DIR}/${DLL_NAME}")
                add_custom_command(TARGET c_chess POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CURL_LIB_DIR}/${DLL_NAME}"
                    $<TARGET_FILE_DIR:c_chess>)
            elseif(EXISTS "${CURL_LIB_DIR}/../bin/${DLL_NAME}")
                message(STATUS "Found additional DLL: ${CURL_LIB_DIR}/../bin/${DLL_NAME}")
                add_custom_command(TARGET c_chess POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CURL_LIB_DIR}/../bin/${DLL_NAME}"
                    $<TARGET_FILE_DIR:c_chess>)
            endif()
        endforeach()
    endif()
else()
    message(STATUS "CURL not found, building without API support")
    message(STATUS "To enable API support, install libcurl and its development files")
    message(STATUS "For Windows:")
    message(STATUS "  - Install libcurl via vcpkg: vcpkg install curl:x64-windows")
    message(STATUS "  - Then use: cmake -DCURL_DIR=C:/path/to/curl ..")
    message(STATUS "For Linux: sudo apt-get install libcurl4-openssl-dev")
    message(STATUS "For macOS: brew install curl")
    
    # Define preprocessor macro to disable API features
    target_compile_definitions(c_chess PRIVATE NO_CURL_SUPPORT)
endif()

# Print a summary of the configuration
message(STATUS "Configuration summary:")
message(STATUS "  API support: ${CURL_FOUND}")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
