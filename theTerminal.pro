#-------------------------------------------------
#
# Project created by QtCreator 2016-04-25T19:14:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theterminal
TEMPLATE = app
LIBS += -lqtermwidget5

SOURCES += main.cpp\
        mainwindow.cpp \
    terminalwidget.cpp \
    about.cpp

HEADERS  += mainwindow.h \
    terminalwidget.h \
    about.h

FORMS    += mainwindow.ui \
    about.ui
