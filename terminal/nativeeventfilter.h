#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QSettings>

class NativeEventFilter : public QObject,
                          public QAbstractNativeEventFilter {
        Q_OBJECT
    public:
        explicit NativeEventFilter(QObject* parent = 0);

    signals:
        void toggleTerminal();
        void keypressCaptureComplete();

    public slots:
        void captureKeyPresses(bool capture);

    private:
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);

        bool capturingKeyPress = false;
        QSettings settings;
};

#endif // NATIVEEVENTFILTER_H
