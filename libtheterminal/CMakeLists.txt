cmake_minimum_required(VERSION 3.16)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)

set(SOURCES
)

set(HEADERS
    libtheterminal_global.h
)

set(PRIVATE_HEADERS

)

add_library(libtheterminal SHARED ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
cntp_init(libtheterminal 20)
set_target_properties(libtheterminal PROPERTIES
        OUTPUT_NAME theterminal
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.theterminal
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libtheterminal PRIVATE Qt::Widgets)
target_compile_definitions(libtheterminal PRIVATE LIBTHETERMINAL_LIBRARY)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libtheterminal.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/libtheterminal.framework/Headers)
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libtheterminal)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libtheterminal)
    set(INCLUDE_INSTALL_DIR ${HEADER_INSTALL_DIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

configure_package_config_file(libtheterminalConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libtheterminalConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtheterminalConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libtheterminalTargets
        FILE libtheterminalTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libtheterminal
        EXPORT libtheterminalTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()

add_subdirectory(common)
add_subdirectory(com/vicr123/theterminal/libtheterminal)
