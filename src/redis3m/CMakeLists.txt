project(redis3m)
cmake_minimum_required(VERSION 2.8)

if(CMAKE_GENERATOR MATCHES ".*(Make|Ninja).*"
    AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel" FORCE)
  message(STATUS "CMAKE_BUILD_TYPE not specified. Using ${CMAKE_BUILD_TYPE} build")
endif()
IF (APPLE)
    add_subdirectory("hiredis")
ENDIF(APPLE)
set(REDIS3M_VERSION_MAJOR 9)
set(REDIS3M_VERSION_MINOR 9)
set(REDIS3M_VERSION_PATCH 9)
set(REDIS3M_VERSION_STRING ${REDIS3M_VERSION_MAJOR}.${REDIS3M_VERSION_MINOR}.${REDIS3M_VERSION_PATCH})
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -std=c++11 -ggdb -pipe -fPIC -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11 -pipe -fPIC -lpthread")
if(CMAKE_BUILD_TYPE STREQUAL Release)
    set(datadir ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
else()
   set(datadir data)
endif()

find_package(Boost COMPONENTS system filesystem unit_test_framework date_time regex REQUIRED)

# Library
include_directories("${PROJECT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIRS})
file(GLOB SRC_FILES "src/*.cpp" "src/utils/*.cpp" "src/patterns/*.cpp")
file(GLOB INCLUDE_FILES "include/redis3m/*.h*" "include/redis3m/utils/*.h" "include/redis3m/patterns/*.h*")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/utils/datadir.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/datadir.cpp @ONLY)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/datadir.cpp ${INCLUDE_FILES})
target_link_libraries(${PROJECT_NAME} hiredis ${Boost_REGEX_LIBRARY} ${Boost_DATE_TIME_LIBRARY})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${REDIS3M_VERSION_STRING}
                                           SOVERSION ${REDIS3M_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELEASE "DATADIR=\"${datadir}\"")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "DATADIR=\"${CMAKE_SOURCE_DIR}/data\"")
