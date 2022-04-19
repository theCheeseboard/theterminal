#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QDir>
#include <QProcessEnvironment>
#include <QWidget>

class CommandPart;

namespace Ui {
    class TerminalWidget;
}

class TerminalPart;
struct TerminalWidgetPrivate;
class TerminalWidget : public QWidget {
        Q_OBJECT

    public:
        explicit TerminalWidget(QString workDir = "", QString cmd = "", QWidget* parent = nullptr);
        ~TerminalWidget();

        QString title();

        void print();

    public slots:
        void copyClipboard();
        void pasteClipboard();
        void toggleShowSearchBar();
        void close();
        void zoomIn();
        void zoomOut();
        void zoom100();

        QDir getWorkingDir();
        QProcessEnvironment getEnv();

    private slots:
        void adjustCurrentTerminal();

    signals:
        void finished();
        void bell(QString message);
        void switchToThis();
        void openNewTerminal(TerminalWidget* terminalWidget);
        void titleChanged();

    private:
        explicit TerminalWidget(TerminalPart* terminal, QWidget* parent = nullptr);
        void initializeAsLegacy(TerminalPart* terminal);

        Ui::TerminalWidget* ui;
        TerminalWidgetPrivate* d;

        void resizeEvent(QResizeEvent* event);
};

#endif // TERMINALWIDGET_H
