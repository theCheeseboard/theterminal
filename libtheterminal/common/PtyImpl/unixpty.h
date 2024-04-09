#ifndef UNIXPTY_H
#define UNIXPTY_H

#include "../abstractpty.h"
#include <QIODevice>

struct UnixPtyPrivate;
class UnixPty : public AbstractPty {
        Q_OBJECT
    public:
        explicit UnixPty(QObject* parent = nullptr);
        virtual ~UnixPty();

        void kill();

    signals:

    private:
        UnixPtyPrivate* d;

        // AbstractPty interface
    public:
        bool start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows);
        bool ready();
        bool setWindowSize(qint16 cols, qint16 rows);
        QStringList runningProcesses();

        // QIODevice interface
    protected:
        qint64 readData(char* data, qint64 maxlen);
        qint64 writeData(const char* data, qint64 len);
        qint64 bytesAvailable() const;
};

#endif // UNIXPTY_H
