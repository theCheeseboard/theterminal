#include "winpty.h"
#include <wrl/wrappers/corewrappers.h>
#include <tlogger.h>
#include <QWinEventNotifier>
#include <QThread>

using namespace Microsoft::WRL::Wrappers;

struct ReadFileWorker : public QThread
{
    Q_OBJECT
public:
    ReadFileWorker(HANDLE hnd, QObject* parent) : QThread(parent), hnd(hnd) {}

    void run() override {
        std::byte buf[1024];
        while (true) {
            DWORD readBytes;
            if (!ReadFile(hnd, buf, sizeof(buf), &readBytes, nullptr)) {
                auto err = GetLastError();
                return;
            }

            emit dataRead(QByteArrayView(buf, readBytes));
            emit dataRead(QByteArray(buf, readBytes));
        }
    }

    HANDLE hnd;

signals:
    void dataRead(const QByteArray& data);
};

struct WriteFileWorker : public QObject
{
    Q_OBJECT
public:
    WriteFileWorker(HANDLE hnd) : QObject(nullptr), hnd(hnd) {}

    quint64 writeData(QByteArrayView data) {
        DWORD bytesWritten;
        WriteFile(hnd, data.constData(), data.length(), &bytesWritten, nullptr);
        return bytesWritten;
    }

private:
    HANDLE hnd;
};

struct WinPtyPrivate
{
    QProcess* runningProcess;
    WriteFileWorker* writeWorker;
    QThread* readThread, *writeThread;
    QByteArray readBuffer;

    STARTUPINFOEX si{};
    QScopedArrayPointer<std::byte> attributeList;
    HPCON hPC{};

    HandleT<HandleTraits::HANDLETraits> outputReadSide, inputWriteSide;

};

WinPty::WinPty(QObject* parent)
    : AbstractPty{parent},
    d(new WinPtyPrivate)
{}

WinPty::~WinPty() {
    ClosePseudoConsole(d->hPC);
    d->readThread->wait();
    d->writeThread->exit();
    d->writeThread->wait();
}

bool WinPty::start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows) {
    HandleT<HandleTraits::HANDLETraits> inputReadSide, outputWriteSide;

    if (!CreatePipe(inputReadSide.GetAddressOf(), d->inputWriteSide.GetAddressOf(), nullptr, 0) ||
        !CreatePipe(d->outputReadSide.GetAddressOf(), outputWriteSide.GetAddressOf(), nullptr, 0))
    {
        tWarn("WinPty") << "Failed to create pipes with error" << (int)GetLastError();
        return false;
    }

    HPCON hPC;
    HRESULT hr = CreatePseudoConsole({ cols, rows }, inputReadSide.Get(), outputWriteSide.Get(), 0, &hPC);
    if (FAILED(hr))
    {
        tWarn("WinPty") << "Failed to create pseudoconsole with error" << (int)GetLastError();
    }

    d->hPC = hPC;

    auto readThread = new ReadFileWorker(d->outputReadSide.Get(), this);
    connect(readThread, &ReadFileWorker::dataRead, this, [this](const QByteArray& data) {
        d->readBuffer.append(data);
        emit readyRead();
    });
    d->readThread = readThread;

    auto writeThread = new QThread(this);
    auto writeWorker = new WriteFileWorker(d->inputWriteSide.Get());
    writeWorker->moveToThread(writeThread);
    connect(writeThread, &QThread::finished, writeWorker, &QObject::deleteLater);
    d->writeWorker = writeWorker;
    d->writeThread = writeThread;

    auto runningProcess = new QProcess(this);
    runningProcess->setWorkingDirectory(workingDirectory);
    runningProcess->setProcessEnvironment(environment);
    runningProcess->setCreateProcessArgumentsModifier([=](QProcess::CreateProcessArguments* args) {
        // CreateProcessArguments.startupInfo holds a STARTUPINFO, but we need to pass STARTUPINFOEX
        // so we'll sneakily overwrite the pointer. since it needs to be valid after we return
        // from this function, we stick it into d
        STARTUPINFOEX& si = d->si;

        si.StartupInfo = {};
        si.StartupInfo.cb = sizeof(STARTUPINFOEX);

        size_t bytesRequired;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &bytesRequired);

        d->attributeList.reset(new std::byte[bytesRequired]);
        si.lpAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(d->attributeList.get());
        if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired))
        {
            tWarn("WinPty") << "Failed to initialize process thread attributes list with error " << (int)GetLastError();
            return false;
        }

        if (!UpdateProcThreadAttribute(si.lpAttributeList,
            0,
            PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
            hPC,
            sizeof(hPC),
            nullptr,
            nullptr))
        {
            tWarn("WinPty") << "Failed to update process thread attributes list with error " << (int)GetLastError();
            return false;
        }

        args->inheritHandles = false;
        args->startupInfo = reinterpret_cast<Q_STARTUPINFO*>(&d->si);
        args->flags = CREATE_UNICODE_ENVIRONMENT | EXTENDED_STARTUPINFO_PRESENT;
    });

    d->runningProcess = runningProcess;
    runningProcess->start(process, {});
    runningProcess->waitForStarted();

    readThread->start();
    writeThread->start();

    this->setOpenMode(QIODevice::ReadWrite);
    return true;
}

bool WinPty::ready() {
    return false;
}

bool WinPty::setWindowSize(qint16 cols, qint16 rows) {
    if (d->hPC != nullptr) {
        auto hr = ResizePseudoConsole(d->hPC, { cols, rows });
        return SUCCEEDED(hr);
     }
    return false;
}

qint64 WinPty::readData(char* data, qint64 maxlen) {
    auto toRead = qMin(maxlen, d->readBuffer.length());
    memcpy(data, d->readBuffer.constData(), toRead);
    d->readBuffer = d->readBuffer.mid(toRead);
    return toRead;
}

qint64 WinPty::writeData(const char* data, qint64 len) {
    QMetaObject::invokeMethod(d->writeWorker, &WriteFileWorker::writeData, QByteArray(data, len));
    return len;
}

QStringList WinPty::runningProcesses() {
    return {};
}

#include "winpty.moc"
