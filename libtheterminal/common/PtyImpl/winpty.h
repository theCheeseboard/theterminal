#ifndef WINPTY_H
#define WINPTY_H

#include "../ipty.h"
#include <QIODevice>

struct WinPtyPrivate;

class WinPty : public QIODevice,
    public IPty {
        Q_OBJECT
    public:
        explicit WinPty(QObject* parent = nullptr);
        ~WinPty();

    signals:

    private:
        QScopedPointer<WinPtyPrivate> d;

        // IPty interface
    public:
        bool start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows);
        bool ready();
        QIODevice *device();
        bool setWindowSize(qint16 cols, qint16 rows);

        // QIODevice interface
    protected:
        qint64 readData(char *data, qint64 maxlen);
        qint64 writeData(const char *data, qint64 len);
};

#endif // WINPTY_H
