include(idl-gen.cmake)

include_directories(
    ${IDL_GEN_ROOT}
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
