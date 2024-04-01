#ifndef IPTY_H
#define IPTY_H

#include <QObject>
#include <QProcessEnvironment>

class IPty {
    public:
        static IPty* createPty(QObject* parent = nullptr);

        virtual bool start(QString process, QProcessEnvironment environment, QString workingDirectory, qint16 cols, qint16 rows) = 0;
        virtual bool ready() = 0;
        virtual QIODevice* device() = 0;
        virtual bool setWindowSize(qint16 cols, qint16 rows) = 0;
};

#endif // IPTY_H
