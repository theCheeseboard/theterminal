#-------------------------------------------------
#
# Project created by QtCreator 2016-04-25T19:14:06
#
#-------------------------------------------------


QT       += core gui thelib
CONFIG   += c++14

unix:!macx {
    QT += x11extras
    LIBS += -lX11

    blueprint {
        TARGET = theterminalb
        DEFINES += "BLUEPRINT"
    } else {
        TARGET = theterminal
    }
}

macx {
    INCLUDEPATH += /usr/local/include /usr/local/include/the-libs
    LIBS += -L/usr/local/lib/ -lthe-libs
    TARGET = theTerminal
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
#LIBS += -ltttermwidget
LIBS += /home/victor/Documents/Apps/build-tttermwidget-Desktop-Default/libtttermwidget.so


SOURCES += main.cpp\
        mainwindow.cpp \
    about.cpp \
    nativeeventfilter.cpp \
    settingswindow.cpp \
    terminalpart.cpp \
    terminalwidget.cpp \
    commandpart.cpp \
    graphicalParts/lscommand.cpp \
    graphicalParts/ttedcommand.cpp \
    terminalstatus.cpp \
    history.cpp \
    terminalcontroller.cpp

unix:!macx {
    SOURCES += dropdown.cpp
}

HEADERS  += mainwindow.h \
    about.h \
    nativeeventfilter.h \
    settingswindow.h \
    terminalpart.h \
    terminalwidget.h \
    commandpart.h \
    graphicalParts/lscommand.h \
    graphicalParts/ttedcommand.h \
    terminalstatus.h \
    history.h \
    terminalcontroller.h

unix:!macx {
    HEADERS += dropdown.h
}

FORMS    += mainwindow.ui \
    about.ui \
    settingswindow.ui \
    terminalwidget.ui \
    commandpart.ui \
    graphicalParts/lscommand.ui \
    graphicalParts/ttedcommand.ui \
    terminalstatus.ui


unix:!macx {
    FORMS += dropdown.ui
}

unix {
    QMAKE_STRIP = echo
    target.path = /usr/bin

    blueprint {
        appentry.path = /usr/share/applications
        appentry.files = theterminalb.desktop
    } else {
        appentry.path = /usr/share/applications
        appentry.files = theterminal.desktop theterminaldd.desktop
    }

    INSTALLS += target appentry
}

DISTFILES += \
    theterminaldd.desktop \
    theterminal.desktop \
    theterminalb.desktop

RESOURCES += \
    icons.qrc
