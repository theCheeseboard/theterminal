#ifndef UNIXPTY_H
#define UNIXPTY_H

#include "../ipty.h"
#include <QIODevice>

struct UnixPtyPrivate;
class UnixPty : public QIODevice,
                public IPty {
        Q_OBJECT
    public:
        explicit UnixPty(QObject* parent = nullptr);
        virtual ~UnixPty();

        void kill();

    signals:

    private:
        UnixPtyPrivate* d;

        // IPty interface
    public:
        bool start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows);
        bool ready();
        QIODevice* device();
        bool setWindowSize(qint16 cols, qint16 rows);

        // QIODevice interface
    protected:
        qint64 readData(char* data, qint64 maxlen);
        qint64 writeData(const char* data, qint64 len);
        qint64 bytesAvailable() const;
};

#endif // UNIXPTY_H
