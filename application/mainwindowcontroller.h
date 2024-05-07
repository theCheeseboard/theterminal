#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QObject>
#include <QQmlEngine>

class MainWindowController : public QObject {
        Q_OBJECT
        QML_ELEMENT
    public:
        explicit MainWindowController(QObject* parent = nullptr);

        Q_SCRIPTABLE void bell();

    signals:
};

#endif // MAINWINDOWCONTROLLER_H
