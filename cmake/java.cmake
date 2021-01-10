link_libraries(common_base)

find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

set(FDBUS_JAVA_SRC_ROOT ${PACKAGE_SOURCE_ROOT}/java/src)
set(FDBUS_JAVA_SRC_DIR ${FDBUS_JAVA_SRC_ROOT}/ipc/fdbus)

file(GLOB JAVA_SRC "${FDBUS_JAVA_SRC_DIR}/*.java")

set(CMAKE_JAVA_INCLUDE_PATH ${FDBUS_JAVA_SRC_DIR})
set(CMAKE_JAVA_COMPILE_FLAGS -Xlint:unchecked -Xlint:deprecation)

add_jar(protobus-jar ${JAVA_SRC}
        INCLUDE_JARS ${CMAKE_INSTALL_PREFIX}/usr/share/fdbus/protobuf.jar ${CMAKE_INSTALL_PREFIX}/usr/share/fdbus/fdbus-jni.jar
        OUTPUT_DIR ${CMAKE_INSTALL_PREFIX}/usr/share/fdbus
        OUTPUT_NAME protobus
        GENERATE_NATIVE_HEADERS protobus-native DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/share/fdbus)

#set(FDBUS_CPP_SRC_ROOT ${PACKAGE_SOURCE_ROOT}/jni/src/cpp)
#file(GLOB JNI_CPP_SRC "${FDBUS_CPP_SRC_ROOT}/*.cpp")
#add_library(fdbus-jni SHARED ${JNI_CPP_SRC})
#target_include_directories(fdbus-jni PUBLIC ${JNI_INCLUDE_DIRS})
##target_link_libraries(fdbus-jni PRIVATE fdbus-native)
#target_link_libraries(fdbus-jni)
#install(TARGETS fdbus-jni DESTINATION usr/lib)
