cmake_minimum_required(VERSION 3.21.0)

project(theterminal VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Quick QuickControls2)
find_package(libcontemporary REQUIRED)

set(SOURCES
        main.cpp

)

set(HEADERS

)

# Find Apple lproj files
#set(LPROJ_SOURCES)
#file(GLOB LPROJ_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/translations/apple-lproj/*.lproj)
#FOREACH(DIR ${LPROJ_DIRS})
#    cmake_path(GET DIR FILENAME DIR_NAME)
#    file(GLOB LPROJ_FILES ${DIR}/*.strings)
#    FOREACH(FILE ${LPROJ_FILES})
#        list(APPEND LPROJ_SOURCES ${FILE})
#        set_source_files_properties(${FILE} PROPERTIES
#                MACOSX_PACKAGE_LOCATION Resources/${DIR_NAME})
#    ENDFOREACH()
#ENDFOREACH()
#set(SOURCES ${SOURCES} ${LPROJ_SOURCES})

qt_add_executable(theterminal ${SOURCES} ${HEADERS})

qt_add_qml_module(theterminal
    URI com.vicr123.theterminal
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        mainwindowcontroller.h mainwindowcontroller.cpp
)

set_target_properties(theterminal PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.theterminal
        WIN32_EXECUTABLE TRUE
        CNTP_DATA_SUBDIR theterminal)

cntp_init(theterminal 20)
cntp_translate(theterminal)
cntp_target_name(theterminal "theTerminal")
cntp_app_icon(theterminal
    BASEICON icons/baseicon.svg
    BASEICON_MAC icons/baseicon-mac.svg)
cntp_target_desktop_file(theterminal
    DESKTOP_FILE com.vicr123.theterminal.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.theterminal_blueprint.desktop)
cntp_defaults_file(theterminal
    DEFAULTS_FILE theterminal.conf)
cntp_dmg(theterminal app-dmg-background.svg)

target_link_libraries(theterminal PUBLIC Qt::Widgets Qt::Quick Qt::QuickControls2 libcontemporary libtheterminal)
target_include_directories(theterminal PUBLIC ../libtheterminal/common/ ../libtheterminal)

install(TARGETS theterminal
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
