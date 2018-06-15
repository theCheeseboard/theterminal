#include "mainwindow.h"
#include <QApplication>

#ifndef Q_OS_MAC
#include "dropdown.h"
#endif

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

            #ifndef Q_OS_MAC
            qDebug() << "  -d[ropdown]                  Starts theTerminal in dropdown mode";
            #endif

            qDebug() << "  -h, --help                   Show this help message";
            return 0;
        } else if (arg.startsWith("-w=")) {
            workDir = arg.remove("-w=");
        } else if (arg.startsWith("-d")) {
            dropdown = true;
        }
    }


#ifdef Q_OS_MAC
    QIcon::setFallbackSearchPaths(QStringList() << ":/");
    QIcon::setThemeName("icons");
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    filter = new NativeEventFilter;
    a.installNativeEventFilter(filter);

    if (dropdown) {
        #ifndef Q_OS_MAC
        Dropdown* w = new Dropdown(workDir);
        #endif
    } else {
        MainWindow* w = new MainWindow(workDir);
        w->show();
    }

    return a.exec();
}


int lookbehindSpace(QString str, int from) {
    bool inQuotes = false;
    for (int i = 0; i < from; i++) {
        if (str.at(i) == '\"') {
            inQuotes = !inQuotes;
        }
    }

    for (int i = from - 1; i >= 0; i--) {
        if (str.at(i) == '\"') {
            inQuotes = !inQuotes;
        } else if (!inQuotes && str.at(i) == ' ') {
            return i;
        }
    }
    return -1;
}

int lookaheadSpace(QString str, int from) {
    bool inQuotes = false;
    for (int i = 0; i < from; i++) {
        if (str.at(i) == '\"') {
            inQuotes = !inQuotes;
        }
    }

    for (int i = from; i < str.length(); i++) {
        if (str.at(i) == '\"') {
            inQuotes = !inQuotes;
        } else if (!inQuotes && str.at(i) == ' ') {
            return i;
        }
    }
    return -1;
}

QStringList splitSpaces(QString str) {
    bool inQuotes = false;
    QString currentString;
    QStringList list;

    for (int i = 0; i < str.length(); i++) {
        if (str.at(i) == '\"') {
            inQuotes = !inQuotes;
        } else if (inQuotes) {
            currentString.append(str.at(i));
        } else {
            if (str.at(i) == ' ') {
                list.append(currentString);
                currentString.clear();
            } else {
                currentString.append(str.at(i));
            }
        }
    }

    if (currentString != "") list.append(currentString);

    return list;
}
