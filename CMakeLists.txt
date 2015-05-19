cmake_minimum_required(VERSION 2.8)
project (leanclub)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Tcmalloc)
find_package(Threads)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()


if (MSVC)
set(Boost_USE_STATIC_LIBS "On")
find_package( Boost 1.52 COMPONENTS date_time filesystem system thread regex random REQUIRED )
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
find_package( Boost 1.52 COMPONENTS date_time filesystem system thread random REQUIRED )
endif()

include_directories( ${Boost_INCLUDE_DIR} )

set(PROJECT_INCLUDE_DIR 
${PROJECT_SOURCE_DIR}/include
)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")
add_subdirectory("src/sundown")
add_subdirectory("src/redis3m")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES
    src/config.h
    src/main.cpp
    src/api.h
    src/api.cpp
    src/utils.h
    src/utils.cpp
    src/pages.h
    src/pages.cpp
    src/oauth/sha1.h
    src/oauth/sha1.cpp
    src/oauth/base64.h
    src/oauth/base64.cpp
    src/oauth/md5.h
    src/oauth/md5.cpp
    src/oauth/urlencode.h
    src/oauth/urlencode.cpp
    src/oauth/liboauthcpp.h
    src/oauth/liboauthcpp.cpp
    src/restclient/meta.h
    src/restclient/restclient.cpp
    src/restclient/restclient.h
	)


add_executable(leanclub ${SOURCE_FILES})
target_link_libraries(leanclub ${Boost_LIBRARIES})
target_link_libraries(leanclub ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(leanclub curl)
target_link_libraries(leanclub redis3m)
target_link_libraries(leanclub hoedown)

add_custom_command(TARGET leanclub PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/templates ${CMAKE_CURRENT_BINARY_DIR}/templates)
add_custom_command(TARGET leanclub PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/static ${CMAKE_CURRENT_BINARY_DIR}/static)