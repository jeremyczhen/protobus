cmake_minimum_required(VERSION 3.1.3)
project(pb-example C CXX)

option(protobus_BUILD_JAVA "Build JAVA" OFF)

if (MSVC)
    add_definitions("-D__WIN32__")
elseif(fdbus_ANDROID)
    add_definitions("-D__LINUX__")
else()
    if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall ${CMAKE_CXX_FLAGS}")
    endif()
    #add_compile_options(-g -O0)
    add_definitions("-D__LINUX__")
endif()

if(DEFINED RULE_DIR)
    include(${RULE_DIR}/rule_base.cmake)
else()
    set(RULE_DIR ${CMAKE_SOURCE_DIR})
    include(rule_base.cmake)
endif()

get_filename_component(PACKAGE_SOURCE_ROOT ${CMAKE_SOURCE_DIR} PATH)
get_filename_component(PACKAGE_SOURCE_ROOT ${PACKAGE_SOURCE_ROOT} PATH)

print_variable(PACKAGE_SOURCE_ROOT)

set(IDL_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)

    link_libraries(ws2_32.lib)
    set(LIB_BUILD_TYPE "STATIC")
else()
    if (fdbus_LINK_PTHREAD_LIB)
        link_libraries(pthread)
    endif()
    if (fdbus_LINK_SOCKET_LIB)
        link_libraries(socket)
    endif()

    set(LIB_BUILD_TYPE "SHARED")
endif()

include(idl-gen.cmake)

include_directories(
    ${IDL_GEN_ROOT}
    ${CMAKE_INSTALL_PREFIX}/usr/include/protobus/idl-gen
    )

add_definitions("-DFDB_IDL_EXAMPLE_H=<idl-gen/fdbus-pb.Example.pb.h>")

add_executable(fdbpbclient
    ${PACKAGE_SOURCE_ROOT}/cpp/example/fdbpbclient.cpp
    ${IDL_GEN_ROOT}/idl-gen/fdbus-pb.Example.pb.cc
)
target_link_libraries(fdbpbclient fdbus-pb common_base)

add_executable(fdbpbserver
    ${PACKAGE_SOURCE_ROOT}/cpp/example/fdbpbserver.cpp
    ${IDL_GEN_ROOT}/idl-gen/fdbus-pb.Example.pb.cc
)
target_link_libraries(fdbpbserver fdbus-pb common_base)
if (MSVC)
    target_link_libraries(fdbpbclient libprotobufd ws2_32.lib)
    target_link_libraries(fdbpbserver libprotobufd ws2_32.lib)
else()
    target_link_libraries(fdbpbclient protobuf)
    target_link_libraries(fdbpbserver protobuf)
endif()

install(TARGETS fdbpbclient fdbpbserver RUNTIME DESTINATION usr/bin)

