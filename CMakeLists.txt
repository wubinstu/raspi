cmake_minimum_required(VERSION 3.10)
project(rain VERSION 1.5.2 LANGUAGES C)

set(SIMULATOR false)
set(CMAKE_INSTALL_PREFIX /usr/local)
set(CMAKE_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_CONF_DIR ${CMAKE_INSTALL_PREFIX}/etc)
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/inc/cmake_conf.h)

set(CMAKE_BUILD_TYPE Debug)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

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
set(LIB_TYPE STATIC)

add_library(filed ${LIB_TYPE} src/filed.c src/global.c src/log.c)
add_library(linklist ${LIB_TYPE} src/linklist.c src/global.c src/types.c)
add_library(raspi_drive ${LIB_TYPE} src/raspi_drive.c src/global.c src/log.c)
add_library(rssl ${LIB_TYPE} src/rssl.c src/global.c src/log.c)
add_library(socket_fd ${LIB_TYPE} src/socket_fd.c src/global.c src/log.c)

add_library(arr ${LIB_TYPE} src/arr.c)
add_library(conf ${LIB_TYPE} src/conf.c)
add_library(load_clean ${LIB_TYPE} src/load_clean.c)
add_library(rsignal ${LIB_TYPE} src/rsignal.c)
add_library(client_run ${LIB_TYPE} src/client_run.c)

# exec
add_executable(rain_client rain_client.c)
#add_executable(test_server test_server.c)
add_executable(rain_server rain_server.c)

#link
if (NOT ${SIMULATOR})
    target_link_libraries(raspi_drive wiringPi)
endif ()
target_link_libraries(rssl crypto ssl)
target_link_libraries(conf arr filed linklist)
target_link_libraries(load_clean conf raspi_drive rssl socket_fd)
target_link_libraries(rsignal load_clean)
target_link_libraries(client_run load_clean)
target_link_libraries(rain_client rsignal client_run pthread)
#target_link_libraries(test_server load_clean)
target_link_libraries(rain_server load_clean)


set(systemdUnit /usr/lib/systemd/system)
install(TARGETS rain_client DESTINATION bin)
install(TARGETS rain_server DESTINATION bin)
install(FILES rain_client.service DESTINATION ${systemdUnit})
install(FILES rain_server.service DESTINATION ${systemdUnit})


message(STATUS "工程代号:灵泽  出自: [晋] 张骏《东门行》: '旱天降灵泽,朝日辉华精' 这是本人首个工程级别项目,祝愿它足够优秀")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
message(STATUS "Build TimeStamp = ${BUILD_TIMESTAMP}")
message(STATUS "Project Name = ${CMAKE_PROJECT_NAME}")
message(STATUS "Project Version = ${CMAKE_PROJECT_VERSION}")
message(STATUS "Project Build Type = ${CMAKE_BUILD_TYPE}")
message(STATUS "Project Source Path = ${CMAKE_SOURCE_DIR}")
message(STATUS "C Language Standard = C${CMAKE_C_STANDARD}")
message(STATUS "Simulator Mode = ${SIMULATOR}")
message(STATUS "Binary OutPut Path = ${PROJECT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "Library Output Path = ${PROJECT_BINARY_DIR}/${LIBRARY_OUTPUT_PATH}")
message(STATUS "Install Path = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Systemd Unit File Path = ${systemdUnit}")
message(STATUS "Configuration File Path = ${CMAKE_INSTALL_PREFIX}/etc/${CMAKE_PROJECT_NAME}.conf")
