#include "mainwindow.h"
#include <tapplication.h>
#include <QCommandLineParser>
#include <tcsdtools.h>

#ifndef Q_OS_MAC
#include "dropdown.h"
#endif

#ifdef Q_OS_MAC
#include <CoreFoundation/CFBundle.h>
#endif

NativeEventFilter* filter;
extern void setupBuiltinFunctions();

int main(int argc, char *argv[])
{
    tApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theTerminal");
    if (QDir("/usr/share/theterminal/").exists()) {
        a.setShareDir("/usr/share/theterminal/");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theterminal/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theterminal/"));
    }
    a.installTranslators();

    QSettings settings;
    tCsdGlobal::setCsdsEnabled(!settings.value("appearance/useSsds", false).toBool());

    QCommandLineParser parser;
    parser.addOptions({
        {{"w","workdir"}, a.translate("main", "Set working directory"), a.translate("main", "workdir")},
        {{"d","dropdown"}, a.translate("main", "Starts theTerminal in dropdown mode")},
        {{"e","exec"}, a.translate("main", "Command to execute"), a.translate("main", "cmd")}
    });
    parser.addHelpOption();
    parser.process(a);


#ifdef Q_OS_MAC
    QIcon::setFallbackSearchPaths(QStringList() << ":/");
    QIcon::setThemeName("icons");
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    filter = new NativeEventFilter;
    a.installNativeEventFilter(filter);

    if (parser.isSet("d")) {
        #ifndef Q_OS_MAC
        Dropdown* w = new Dropdown(parser.value("workdir"));
        #endif
    } else {
        MainWindow* w = new MainWindow(parser.value("workdir"), parser.value("exec"));
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
