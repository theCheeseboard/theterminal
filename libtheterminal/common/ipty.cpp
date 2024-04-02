#include "ipty.h"

#include "PtyImpl/unixpty.h"
#include "PtyImpl/winpty.h"

IPty* IPty::createPty(QObject* parent) {
#ifdef Q_OS_WIN
    return new WinPty(parent);
#else
    return new UnixPty(parent);
#endif
    return nullptr;
}
