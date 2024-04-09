#include "abstractpty.h"

#include "PtyImpl/unixpty.h"
#include "PtyImpl/winpty.h"

AbstractPty* AbstractPty::createPty(QObject* parent) {
#ifdef Q_OS_WIN
    return new WinPty(parent);
#else
    return new UnixPty(parent);
#endif
    return nullptr;
}

AbstractPty::AbstractPty(QObject* parent) :
    QIODevice{parent} {
}
