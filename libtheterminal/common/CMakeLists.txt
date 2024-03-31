qt_add_library(libtheterminal-common SHARED
    PtyImpl/unixpty.h PtyImpl/unixpty.cpp
    ipty.h ipty.cpp
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
