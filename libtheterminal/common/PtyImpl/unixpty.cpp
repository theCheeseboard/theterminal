#include "unixpty.h"
#include <QQueue>
#include <QSocketNotifier>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <tlogger.h>
#include <unistd.h>
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_FREEBSD)
    #include <utmpx.h>
#endif

#ifdef Q_OS_MAC
    #include <libproc.h>
#endif

struct UnixPtyPrivate {
        int ptyMaster = -1;
        int ptySlave = -1;
        QString slaveName;
        bool ptyReady = false;
        QProcess* runningProcess = nullptr;
        QSocketNotifier* readNotifier = nullptr;
        QSize size;

        QByteArray readBuffer;
};

UnixPty::UnixPty(QObject* parent) :
    AbstractPty{parent}, d{new UnixPtyPrivate} {
    tDebug("UnixPty") << "Creating UNIX Pty";
}

UnixPty::~UnixPty() {
    kill();
    delete d;
}

void UnixPty::kill() {
    d->slaveName = QString();
    if (d->ptySlave >= 0) {
        ::close(d->ptySlave);
        d->ptySlave = -1;
    }
    if (d->ptyMaster >= 0) {
        ::close(d->ptyMaster);
        d->ptyMaster = -1;
    }

    this->setOpenMode(QIODevice::NotOpen);

    if (d->runningProcess) {
        if (d->runningProcess->state() == QProcess::Running) {
            d->readNotifier->disconnect();
            d->readNotifier->deleteLater();

            d->runningProcess->terminate();
        }
    }
}

bool UnixPty::start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows) {
    d->ptyMaster = ::posix_openpt(O_RDWR | O_NOCTTY);
    if (d->ptyMaster <= 0) {
        tWarn("UnixPty") << "Unable to open master PTY: " << strerror(errno);
        kill();
        return false;
    }

    d->slaveName = ::ptsname(d->ptyMaster);
    if (d->slaveName.isEmpty()) {
        tWarn("UnixPty") << "Unable to get slave name: " << strerror(errno);
        kill();
        return false;
    }

    if (::grantpt(d->ptyMaster) != 0) {
        tWarn("UnixPty") << "Unable to set slave permissions: " << strerror(errno);
        kill();
        return false;
    }

    if (::unlockpt(d->ptyMaster) != 0) {
        tWarn("UnixPty") << "Unable to unlock slave: " << strerror(errno);
        kill();
        return false;
    }

    d->ptySlave = ::open(d->slaveName.toLatin1().data(), O_RDWR | O_NOCTTY);
    if (d->ptySlave < 0) {
        tWarn("UnixPty") << "Unable to open slave: " << strerror(errno);
        kill();
        return false;
    }

    if (fcntl(d->ptyMaster, F_SETFD, FD_CLOEXEC) == -1) {
        tWarn("UnixPty") << "Unable to set master pty flags: " << strerror(errno);
        kill();
        return false;
    }

    if (fcntl(d->ptySlave, F_SETFD, FD_CLOEXEC) == -1) {
        tWarn("UnixPty") << "Unable to set slave pty flags: " << strerror(errno);
        kill();
        return false;
    }

    struct ::termios ttmode;
    if (::tcgetattr(d->ptyMaster, &ttmode) != 0) {
        tWarn("UnixPty") << "Unable to get termios settings for master pty: " << strerror(errno);
        kill();
        return false;
    }

    ttmode.c_iflag = ICRNL | IXON | IXANY | IMAXBEL | BRKINT;
#if defined(IUTF8)
    ttmode.c_iflag |= IUTF8;
#endif

    ttmode.c_oflag = OPOST | ONLCR;
    ttmode.c_cflag = CREAD | CS8 | HUPCL;
    ttmode.c_lflag = ICANON | ISIG | IEXTEN | ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL;

    ttmode.c_cc[VEOF] = 4;
    ttmode.c_cc[VEOL] = -1;
    ttmode.c_cc[VEOL2] = -1;
    ttmode.c_cc[VERASE] = 0x7f;
    ttmode.c_cc[VWERASE] = 23;
    ttmode.c_cc[VKILL] = 21;
    ttmode.c_cc[VREPRINT] = 18;
    ttmode.c_cc[VINTR] = 3;
    ttmode.c_cc[VQUIT] = 0x1c;
    ttmode.c_cc[VSUSP] = 26;
    ttmode.c_cc[VSTART] = 17;
    ttmode.c_cc[VSTOP] = 19;
    ttmode.c_cc[VLNEXT] = 22;
    ttmode.c_cc[VDISCARD] = 15;
    ttmode.c_cc[VMIN] = 1;
    ttmode.c_cc[VTIME] = 0;

#if (__APPLE__)
    ttmode.c_cc[VDSUSP] = 25;
    ttmode.c_cc[VSTATUS] = 20;
#endif

    cfsetispeed(&ttmode, B38400);
    cfsetospeed(&ttmode, B38400);

    if (::tcsetattr(d->ptyMaster, TCSANOW, &ttmode) != 0) {
        tWarn("UnixPty") << "Unable to set termios settings for master pty: " << strerror(errno);
        kill();
        return false;
    }

    d->runningProcess = new QProcess(this);

    d->readNotifier = new QSocketNotifier(d->ptyMaster, QSocketNotifier::Read, d->runningProcess);
    d->readNotifier->setEnabled(true);
    QObject::connect(d->readNotifier, &QSocketNotifier::activated, this, [this](int socket) {
        Q_UNUSED(socket)

        int size = 1025;
        int readSize = 1024;
        char nativeBuffer[size];
        int readBytes = 0;
        do {
            readBytes = ::read(d->ptyMaster, nativeBuffer, readSize);
            d->readBuffer.append(nativeBuffer, readBytes);
        } while (readBytes == readSize); // last data block always < readSize

        emit readyRead();
    });

    QProcessEnvironment finalEnv;
    finalEnv.insert("TERM", "xterm-256color");
    finalEnv.insert(environment);

    d->runningProcess->setWorkingDirectory(workingDirectory);
    d->runningProcess->setProcessEnvironment(finalEnv);
    d->runningProcess->setReadChannel(QProcess::StandardOutput);
    d->runningProcess->setChildProcessModifier([this] {
        dup2(d->ptySlave, STDIN_FILENO);
        dup2(d->ptySlave, STDOUT_FILENO);
        dup2(d->ptySlave, STDERR_FILENO);

        pid_t sid = setsid();
        ioctl(d->ptySlave, TIOCSCTTY, 0);
        tcsetpgrp(d->ptySlave, sid);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_FREEBSD)
        // on Android imposible to put record to the 'utmp' file
        struct utmpx utmpxInfo;
        memset(&utmpxInfo, 0, sizeof(utmpxInfo));

        strncpy(utmpxInfo.ut_user, qgetenv("USER"), sizeof(utmpxInfo.ut_user));

        auto device = d->slaveName.toLatin1();
        if (device.startsWith("/dev/"))
            device = device.mid(5);

        auto d = device.constData();

        strncpy(utmpxInfo.ut_line, d, sizeof(utmpxInfo.ut_line));
        strncpy(utmpxInfo.ut_id, d + strlen(d) - sizeof(utmpxInfo.ut_id), sizeof(utmpxInfo.ut_id));

        struct timeval tv;
        gettimeofday(&tv, 0);
        utmpxInfo.ut_tv.tv_sec = tv.tv_sec;
        utmpxInfo.ut_tv.tv_usec = tv.tv_usec;

        utmpxInfo.ut_type = USER_PROCESS;
        utmpxInfo.ut_pid = getpid();

        utmpxname(_PATH_UTMPX);
        setutxent();
        pututxline(&utmpxInfo);
        endutxent();

    #if !defined(Q_OS_UNIX)
        updwtmpx(_PATH_UTMPX, &loginInfo);
    #endif

#endif
    });
    d->runningProcess->start(process, QStringList());
    d->runningProcess->waitForStarted();

    connect(d->runningProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        d->ptyReady = false;
        emit processQuit(exitCode);
    });

    setWindowSize(cols, rows);

    // m_pid = m_shellProcess.processId();

    this->setOpenMode(QIODevice::ReadWrite);
    d->ptyReady = true;

    return true;
}

bool UnixPty::ready() {
    return d->ptyReady;
}

bool UnixPty::setWindowSize(qint16 cols, qint16 rows) {
    struct winsize winp;
    winp.ws_col = cols;
    winp.ws_row = rows;
    winp.ws_xpixel = 0;
    winp.ws_ypixel = 0;

    auto masterIoctl = ioctl(d->ptyMaster, TIOCSWINSZ, &winp);
    auto slaveIoctl = ioctl(d->ptySlave, TIOCSWINSZ, &winp);
    bool ok = (masterIoctl != -1) && (slaveIoctl != -1);

    if (ok) {
        d->size = {cols, rows};
    }

    return ok;
}

QStringList UnixPty::runningProcesses() {
    if (!d->ptyReady) return {};

#ifdef Q_OS_MAC
    QQueue<int> pids;
    pids.enqueue(d->runningProcess->processId());

    QStringList processes;

    while (!pids.isEmpty()) {
        auto pid = pids.dequeue();
        proc_bsdinfo procInfo;
        if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &procInfo, sizeof(procInfo)) <= 0) {
            continue;
        }

        processes.append(QString(procInfo.pbi_name));

        int numberOfProcesses = proc_listallpids(nullptr, 0);
        int arrayOfPids[numberOfProcesses];
        numberOfProcesses = proc_listallpids(arrayOfPids, sizeof(arrayOfPids));

        for (int i = 0; i < numberOfProcesses; i++) {
            proc_bsdinfo childProcInfo;
            if (proc_pidinfo(arrayOfPids[i], PROC_PIDTBSDINFO, 0, &childProcInfo, sizeof(childProcInfo)) > 0) {
                if (childProcInfo.pbi_ppid == pid) {
                    pids.enqueue(arrayOfPids[i]);
                }
            }
        }
    }

    return processes;
#endif
    return {};
}

qint64 UnixPty::readData(char* data, qint64 maxlen) {
    auto toRead = qMin(maxlen, d->readBuffer.length());
    memcpy(data, d->readBuffer.constData(), toRead);
    d->readBuffer = d->readBuffer.mid(toRead);
    return toRead;
}

qint64 UnixPty::writeData(const char* data, qint64 len) {
    ::write(d->ptyMaster, data, len);
    return len;
}

qint64 UnixPty::bytesAvailable() const {
    return d->readBuffer.length();
}
