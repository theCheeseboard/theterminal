cmake_minimum_required(VERSION 3.16)

cntp_add_qml_module(qml libtheterminal
    URI com.vicr123.theterminal.libtheterminal
    QML_FILES
        Terminal.qml
    SOURCES
)

add_subdirectory(impl)
