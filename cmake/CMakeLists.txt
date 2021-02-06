cmake_minimum_required(VERSION 3.1.3)
project(protobus C CXX)

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

include(cpp.cmake)

if (protobus_BUILD_JAVA)
    include(java.cmake)
endif()

install(FILES ${PACKAGE_SOURCE_ROOT}/python/protobus.py DESTINATION usr/share/fdbus)

#set( CMAKE_VERBOSE_MAKEFILE on )

print_variable(protobus_BUILD_JAVA)
