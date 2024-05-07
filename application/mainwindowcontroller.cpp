#include "mainwindowcontroller.h"

#include <QApplication>

MainWindowController::MainWindowController(QObject* parent) :
    QObject{parent} {
}

void MainWindowController::bell() {
    QApplication::beep();
}
