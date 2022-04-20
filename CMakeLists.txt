cmake_minimum_required(VERSION 3.0)
project(rain VERSION 1.4 LANGUAGES C)

set(SIMULATOR false)
set(CMAKE_INSTALL_PREFIX /usr/local)
set(CMAKE_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_CONF_DIR ${CMAKE_INSTALL_PREFIX}/etc)
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/inc/cmake_conf.h)

#set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

set(CMAKE_C_STANDARD 11)
add_compile_options(-std=gnu11)
set(CMAKE_VERBOSE_MAKEFILE no)
set_property(GLOBAL PROPERTY RULE_MESSAGES ON)

# 设定目录含义
#set(CMAKE_SOURCE_DIR src)
set(EXECUTABLE_OUTPUT_PATH bin)
include_directories(inc)
#target_include_directories(inc)
set(LIBRARY_OUTPUT_PATH lib)
link_directories(lib)
#link_directories(../lib)



# add lib
add_library(arr STATIC src/arr.c)
add_library(conf STATIC src/conf.c)
add_library(file STATIC src/file.c)
add_library(filed STATIC src/filed.c)
add_library(msg STATIC src/msg.c)
add_library(msgd STATIC src/msgd.c)
add_library(mylink STATIC src/mylink.c)
add_library(myprocess STATIC src/myprocess.c)
add_library(mysock STATIC src/mysock.c)
add_library(mysockd STATIC src/mysockd.c)
add_library(myssl STATIC src/myssl.c)
add_library(mythread STATIC src/mythread.c)
add_library(raspi STATIC src/raspi.c)
add_library(sundry STATIC src/sundry.c)
add_library(types STATIC src/types.c)

# Library dependency
target_link_libraries(conf arr filed mylink)
target_link_libraries(file msg)
target_link_libraries(filed msgd)
target_link_libraries(mylink types)
target_link_libraries(myprocess msgd myssl raspi)
target_link_libraries(mysock msg)
target_link_libraries(mysockd msgd)
target_link_libraries(myssl crypto msgd ssl)
target_link_libraries(mythread conf msgd myprocess mysockd myssl raspi types)
target_link_libraries(raspi msgd)

if(NOT ${SIMULATOR})
target_link_libraries(raspi wiringPi)
endif()


# exec
add_executable(raind raind.c)
add_executable(test_server test_server.c)
#add_executable(rain rain.c)


target_link_libraries(raind conf msgd myprocess mysockd mythread myssl pthread raspi)
target_link_libraries(test_server msg mysockd myssl types)


set(systemdUnit /usr/lib/systemd/system)
install(TARGETS raind DESTINATION bin)
install(FILES raind.service DESTINATION ${systemdUnit})


message(STATUS "工程代号:灵泽  出自: [晋] 张骏《东门行》: '旱天降灵泽,朝日辉华精'")
message(STATUS "Project Name = ${CMAKE_PROJECT_NAME}")
message(STATUS "Project Version = ${CMAKE_PROJECT_VERSION}")
message(STATUS "Project Build Type = ${CMAKE_BUILD_TYPE}")
message(STATUS "Project Source Path = ${CMAKE_SOURCE_DIR}")
message(STATUS "Compiler Standard = ${CMAKE_C_STANDARD}")
message(STATUS "Simulator Mode = ${SIMULATOR}")
message(STATUS "Binary OutPut Path = ${PROJECT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "Library Output Path = ${PROJECT_BINARY_DIR}/${LIBRARY_OUTPUT_PATH}")
message(STATUS "Install Path = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Systemd Unit File Path = ${systemdUnit}")
message(STATUS "Configuration File Path = ${CMAKE_INSTALL_PREFIX}/etc/${CMAKE_PROJECT_NAME}.conf")
