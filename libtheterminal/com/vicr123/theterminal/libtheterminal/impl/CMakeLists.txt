cmake_minimum_required(VERSION 3.16)

cntp_add_qml_module(qml-impl libtheterminal-qmlmodule-qml
    URI com.vicr123.theterminal.libtheterminal.impl
    QML_FILES
        TerminalScreenRun.qml
    SOURCES
        qmlterminalscreencontroller.h qmlterminalscreencontroller.cpp
)

target_link_libraries(libtheterminal-qmlmodule-qml-qmlmodule-qml-impl PRIVATE libtheterminal-common)
target_include_directories(libtheterminal-qmlmodule-qml-qmlmodule-qml-impl PRIVATE ../../../../../common)
