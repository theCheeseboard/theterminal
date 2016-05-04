#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString workDir;

    QStringList args = a.arguments();
    args.removeFirst();
    for (QString arg : a.arguments()) {
        if (arg == "-h" || arg == "--help") {
            qDebug() << "theTerminal";
            qDebug() << "Usage: theterminal [OPTIONS]";
            qDebug() << "  -w=[WORKDIR]                 Set Working directory to [WORKDIR]";
            qDebug() << "  -h, --help                   Show this help message";
            return 0;
        } else if (arg.startsWith("-w=")) {
            workDir = arg.remove("-w=");
        }
    }

    MainWindow w(workDir);
    w.show();

    return a.exec();
}
