cmake_minimum_required(VERSION 3.22)

project(server)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig)

pkg_check_modules(thrift REQUIRED thrift_c_glib)
pkg_check_modules(pcsc REQUIRED libpcsclite)

add_executable(${PROJECT_NAME} main.c
                               gen-c_glib/ogon_types.c
                               gen-c_glib/ogon.c)

target_link_libraries(${PROJECT_NAME} ${thrift_LIBRARIES} ${pcsc_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${thrift_INCLUDE_DIRS} ${pcsc_INCLUDE_DIRS})