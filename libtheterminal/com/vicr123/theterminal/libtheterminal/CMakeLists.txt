cmake_minimum_required(VERSION 3.16)

cntp_add_qml_module(qml libtheterminal
    URI com.vicr123.theterminal.libtheterminal
    QML_FILES
        Terminal.qml
    SOURCES
)

target_sources(libtheterminal-qmlmodule-qml
  PRIVATE
    terminalprofile.h terminalprofile.cpp
)

target_link_libraries(libtheterminal-qmlmodule-qml PRIVATE libtheterminal-common)

add_subdirectory(impl)
