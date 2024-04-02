#include "winpty.h"

WinPty::WinPty(QObject* parent)
    : QIODevice{parent}
{}


bool WinPty::start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows) {
    return false;
}

bool WinPty::ready() {
    return false;
}

QIODevice* WinPty::device() {
    return this;
}

bool WinPty::setWindowSize(qint16 cols, qint16 rows) {
    return false;
}

qint64 WinPty::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 WinPty::writeData(const char* data, qint64 len) {
    return len;
}
