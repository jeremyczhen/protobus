option(fdbus_BUILD_CPP_EXAMPLE "Build CPP example" ON)

include_directories(
    ${PACKAGE_SOURCE_ROOT}/cpp
    ${PACKAGE_SOURCE_ROOT}/cpp/public
    )

file(GLOB IMP_SOURCES "${PACKAGE_SOURCE_ROOT}/cpp/imp/*.cpp")

add_library(fdbus-pb ${LIB_BUILD_TYPE} ${IMP_SOURCES})
target_link_libraries(fdbus-pb common_base)
if (MSVC)
    target_link_libraries(fdbus-pb libprotobufd)
else()
    target_link_libraries(fdbus-pb protobuf)
endif()

install(TARGETS fdbus-pb DESTINATION usr/lib)
install(DIRECTORY ${PACKAGE_SOURCE_ROOT}/cpp/public/protobus/ DESTINATION usr/include/protobus)

if (fdbus_BUILD_CPP_EXAMPLE)
    include(example.cmake)
endif()

print_variable(fdbus_BUILD_CPP_EXAMPLE)
