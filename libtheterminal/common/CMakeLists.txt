set(SOURCES)

cntp_sourcelist(
    OUTPUT SOURCES
    OUTPUT_ALL ALL_SOURCES
    FILES
        ipty.h ipty.cpp
        terminalscreen.h terminalscreen.cpp
        Emulation/vt100emulation.h Emulation/vt100emulation.cpp
        StateMachine/terminalstatemachine.h StateMachine/terminalstatemachine.cpp
)

cntp_sourcelist(
    OUTPUT SOURCES
    OUTPUT_ALL ALL_SOURCES
    TARGET_SYSTEM_NAME "Windows"
    FILES
        PtyImpl/winpty.h PtyImpl/winpty.cpp
)

cntp_sourcelist(
    OUTPUT SOURCES
    OUTPUT_ALL ALL_SOURCES
    TARGET_SYSTEM_NAME "Darwin"
    FILES
        PtyImpl/unixpty.h PtyImpl/unixpty.cpp
)

cntp_sourcelist(
    OUTPUT SOURCES
    OUTPUT_ALL ALL_SOURCES
    TARGET_SYSTEM_NAME "Linux"
    FILES
        PtyImpl/unixpty.h PtyImpl/unixpty.cpp
)

qt_add_library(libtheterminal-common SHARED
    ${SOURCES}
    libtheterminal-common-exports.h
    screencolormanager.h screencolormanager.cpp
)

set_target_properties(libtheterminal-common PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(libtheterminal-common PRIVATE LIBTHETERMINAL_COMMON_LIBRARY)

cntp_init(libtheterminal-common 20)

qt_add_resources(libtheterminal-common libtheterminal_colorschemes
    PREFIX /com/vicr123/theterminal/libtheterminal/colorschemes
    BASE colorschemes
    FILES
        colorschemes/GreenOnBlack.colorscheme
        colorschemes/Linux.colorscheme
        colorschemes/Solarized.colorscheme
        colorschemes/SolarizedLight.colorscheme
        colorschemes/Ubuntu.colorscheme
)

install(EXPORT libtheterminalCommonTargets
        FILE libtheterminalCommonTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libtheterminal-common
        EXPORT libtheterminalCommonTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_link_libraries(libtheterminal-common PRIVATE Qt::Core libcontemporary)
target_link_libraries(libtheterminal PUBLIC libtheterminal-common)
