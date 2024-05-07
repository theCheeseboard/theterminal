cmake_minimum_required(VERSION 3.16)

cntp_add_qml_module(qml libtheterminal
    URI com.vicr123.theterminal.libtheterminal
    QML_FILES
        Terminal.qml
    SOURCES
    terminalprofile.h terminalprofile.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_sources(libtheterminal-qmlmodule-qml PRIVATE
        terminalprofile-objc.mm
    )

    find_library(FRAMEWORK_OpenDirectory OpenDirectory)
    target_link_libraries(libtheterminal-qmlmodule-qml PRIVATE ${FRAMEWORK_OpenDirectory})
endif()

target_link_libraries(libtheterminal-qmlmodule-qml PRIVATE libtheterminal-common)

add_subdirectory(impl)
