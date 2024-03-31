#include "ipty.h"

#include "PtyImpl/unixpty.h"

IPty *IPty::createPty(QObject *parent)
{
    return new UnixPty(parent);
    return nullptr;
}
