cmake_minimum_required(VERSION 3.10)
project(rain VERSION 1.7 LANGUAGES C)

set(SIMULATOR true)
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
link_directories(lib /usr/lib/ /usr/lib64/mysql)

find_package(OpenSSL REQUIRED)

# add lib
set(LIB_TYPE STATIC)

add_library(arr ${LIB_TYPE} src/arr.c)
add_library(conf ${LIB_TYPE} src/conf.c src/types.c)
add_library(filed ${LIB_TYPE} src/filed.c src/global.c src/log.c)
add_library(load_clean ${LIB_TYPE} src/load_clean.c)
add_library(raspi_drive ${LIB_TYPE} src/raspi_drive.c src/global.c src/log.c)
add_library(rsignal ${LIB_TYPE} src/rsignal.c)
add_library(rssl ${LIB_TYPE} src/rssl.c src/global.c src/log.c)
add_library(run_client ${LIB_TYPE} src/run_client.c)
add_library(run_server ${LIB_TYPE} src/run_server.c)
add_library(self_sql ${LIB_TYPE} src/log.c src/self_sql.c)
add_library(self_thread ${LIB_TYPE} src/log.c src/self_thread.c)
add_library(socket_fd ${LIB_TYPE} src/socket_fd.c src/global.c src/log.c)
add_library(sql_pool ${LIB_TYPE} src/sql_pool.c)
add_library(ssl_pool ${LIB_TYPE} src/log.c src/ssl_pool.c)
add_library(thread_pool ${LIB_TYPE} src/thread_pool.c)
add_library(web_http ${LIB_TYPE} src/web_http.c)


# exec
add_executable(rain_client src/rain_client.c)
add_executable(rain_server src/rain_server.c)
add_executable(multi_client_test src/multi_client_test.c)

#link
if (NOT ${SIMULATOR})
    target_link_libraries(raspi_drive wiringPi)
endif ()

target_link_libraries(rssl crypto ssl)
target_link_libraries(conf arr filed)
target_link_libraries(load_clean conf raspi_drive rssl socket_fd sql_pool ssl_pool thread_pool)
target_link_libraries(rsignal load_clean)
target_link_libraries(run_client load_clean uuid)
target_link_libraries(run_server load_clean uuid)

target_link_libraries(self_sql mysqlclient)
target_link_libraries(self_thread pthread)
target_link_libraries(ssl_pool crypto ssl self_thread)
target_link_libraries(sql_pool self_sql self_thread)
target_link_libraries(thread_pool self_thread)
target_link_libraries(web_http arr)


target_link_libraries(rain_client rsignal run_client)
target_link_libraries(rain_server rsignal run_server web_http)


set(systemdUnit /usr/lib/systemd/system)
install(TARGETS rain_client DESTINATION bin)
install(TARGETS rain_server DESTINATION bin)
install(FILES rain_client.service DESTINATION ${systemdUnit})
install(FILES rain_server.service DESTINATION ${systemdUnit})


message(STATUS "工程代号:灵泽  出自: [晋] 张骏《东门行》: '旱天降灵泽,朝日辉华精' 这是本人首个工程级别项目,祝愿它人模狗样")
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
message(STATUS "Configuration File Path = ${CMAKE_INSTALL_PREFIX}/etc/${CMAKE_PROJECT_NAME}_*.conf")
