#ifndef ABSTRACTPTY_H
#define ABSTRACTPTY_H

#include "libtheterminal-common-exports.h"
#include <QObject>
#include <QProcessEnvironment>

class LIBTHETERMINAL_COMMON_EXPORT AbstractPty : public QIODevice {
        Q_OBJECT
    public:
        static AbstractPty* createPty(QObject* parent = nullptr);

        virtual bool start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows) = 0;
        virtual bool ready() = 0;
        virtual bool setWindowSize(qint16 cols, qint16 rows) = 0;
        virtual QStringList runningProcesses() = 0;

    protected:
        explicit AbstractPty(QObject* parent = nullptr);

    signals:
        void processQuit(int exitCode);
};

#endif // ABSTRACTPTY_H
