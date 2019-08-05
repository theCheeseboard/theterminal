#-------------------------------------------------
#
# Project created by QtCreator 2016-04-25T19:14:06
#
#-------------------------------------------------


QT       += core gui thelib
CONFIG   += c++14
SHARE_APP_NAME = theterminal

unix:!macx {
    # Include the-libs build scripts
    include(/usr/share/the-libs/pri/buildmaster.pri)
    QT += x11extras tttermwidget
    LIBS += -lX11

    blueprint {
        TARGET = theterminalb
        DEFINES += "BLUEPRINT"
    } else {
        TARGET = theterminal
    }

    FORMS += dropdown.ui

    QMAKE_STRIP = echo
    target.path = /usr/bin

    blueprint {
        appentry.path = /usr/share/applications
        appentry.files = com.vicr123.theterminal-blueprint.desktop
    } else {
        appentry.path = /usr/share/applications
        appentry.files = com.vicr123.theterminal.desktop com.vicr123.theterminal.dropdown.desktop
    }

    INSTALLS += target appentry
}

macx {
    # Include the-libs build scripts
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    LIBS += -F/usr/local/Frameworks/ -framework tttermwidget
    INCLUDEPATH += /usr/local/include /usr/local/include/the-libs /usr/local/Frameworks/tttermwidget.framework/Headers

    blueprint {
        TARGET = "theTerminal Blueprint"
        ICON = icon-bp.icns

        QMAKE_POST_LINK += $$quote(install_name_tool -change tttermwidget.framework/Versions/1/tttermwidget @executable_path/../Frameworks/tttermwidget.framework/Versions/1/tttermwidget $${OUT_PWD}/theTerminal\\ Blueprint.app/Contents/MacOS/theTerminal\\ Blueprint)

    } else {
        TARGET = "theTerminal"
        ICON = icon.icns

        QMAKE_POST_LINK += $$quote(install_name_tool -change tttermwidget.framework/Versions/1/tttermwidget @executable_path/../Frameworks/tttermwidget.framework/Versions/1/tttermwidget $${OUT_PWD}/theTerminal.app/Contents/MacOS/theTerminal)
    }

    tttermwidget.files = /usr/local/Frameworks/tttermwidget.framework
    tttermwidget.path = Contents/Frameworks

    QMAKE_BUNDLE_DATA += tttermwidget
}

win32 {
    # Include the-libs build scripts
    include(C:/Program Files/thelibs/pri/buildmaster.pri)
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    about.cpp \
    models/colorschemeselectiondelegate.cpp \
    nativeeventfilter.cpp \
    settingswindow.cpp \
    terminalpart.cpp \
    terminaltabber.cpp \
    terminalwidget.cpp \
    commandpart.cpp \
    graphicalParts/lscommand.cpp \
    graphicalParts/ttedcommand.cpp \
    terminalstatus.cpp \
    history.cpp \
    terminalcontroller.cpp \
    dialogs/busydialog.cpp

unix:!macx {
    SOURCES += dropdown.cpp
}

HEADERS  += mainwindow.h \
    about.h \
    models/colorschemeselectiondelegate.h \
    nativeeventfilter.h \
    settingswindow.h \
    terminalpart.h \
    terminaltabber.h \
    terminalwidget.h \
    commandpart.h \
    graphicalParts/lscommand.h \
    graphicalParts/ttedcommand.h \
    terminalstatus.h \
    history.h \
    terminalcontroller.h \
    dialogs/busydialog.h

unix:!macx {
    HEADERS += dropdown.h
}

FORMS    += mainwindow.ui \
    about.ui \
    settingswindow.ui \
    terminaltabber.ui \
    terminalwidget.ui \
    commandpart.ui \
    graphicalParts/lscommand.ui \
    graphicalParts/ttedcommand.ui \
    terminalstatus.ui \
    dialogs/busydialog.ui

DISTFILES += \
    com.vicr123.theterminal-blueprint.desktop \
    com.vicr123.theterminal.desktop \
    com.vicr123.theterminal.dropdown.desktop

RESOURCES += \
    icons.qrc
