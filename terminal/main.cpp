#include "mainwindow.h"
#include "dropdown.h"
#include <QApplication>

NativeEventFilter* filter;
extern void setupBuiltinFunctions();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theTerminal");

    QString workDir;
    bool dropdown = false;

    QStringList args = a.arguments();
    args.removeFirst();
    for (QString arg : a.arguments()) {
        if (arg == "-h" || arg == "--help") {
            qDebug() << "theTerminal";
            qDebug() << "Usage: theterminal [OPTIONS]";
            qDebug() << "  -w=[WORKDIR]                 Set Working directory to [WORKDIR]";
            qDebug() << "  -d[ropdown]                  Starts theTerminal in dropdown mode";
            qDebug() << "  -h, --help                   Show this help message";
            return 0;
        } else if (arg.startsWith("-w=")) {
            workDir = arg.remove("-w=");
        } else if (arg.startsWith("-d")) {
            dropdown = true;
        }
    }

    filter = new NativeEventFilter;
    a.installNativeEventFilter(filter);

    if (dropdown) {
        Dropdown* w = new Dropdown(workDir);
    } else {
        MainWindow* w = new MainWindow(workDir);
        w->show();
    }

    return a.exec();
}
