#include "mainwindow.h"
#include "nativeeventfilter.h"
#include <QCommandLineParser>
#include <QDir>
#include <tapplication.h>
#include <tcsdtools.h>
#include <tsettings.h>
#include <tstylemanager.h>

#ifndef Q_OS_MAC
    #include "dropdown.h"
#endif

NativeEventFilter* filter;
extern void setupBuiltinFunctions();

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);

    if (QDir("/usr/share/theterminal/").exists()) {
        a.setShareDir("/usr/share/theterminal/");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theterminal/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theterminal/"));
    }
    a.installTranslators();

    a.setApplicationIcon(QIcon::fromTheme("theterminal", QIcon(":/icons/icon.svg")));
    a.setApplicationVersion("4.0");
    a.setGenericName(QApplication::translate("main", "Terminal"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2022");
    a.setApplicationUrl(tApplication::Sources, QUrl("https://github.com/vicr123/theterminal"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("https://github.com/vicr123/theterminal/issues"));
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

    a.registerCrashTrap();

    tSettings settings;
    //#ifndef Q_OS_MAC
    // Never use CSDs on macOS
    tCsdGlobal::setCsdsEnabled(!settings.value("appearance/useSsds").toBool());
    //#endif

    QCommandLineParser parser;
    parser.addOptions({
        {{"w", "workdir"},       a.translate("main", "Set working directory"),               a.translate("main", "workdir")},
        {{"d", "dropdown"},      a.translate("main", "Starts theTerminal in dropdown mode")},
        {{"e", "exec"}, a.translate("main", "Command to execute"),                                 a.translate("main",                   "cmd")                }
    });
    parser.addHelpOption();
    parser.process(a);

    QObject::connect(&settings, &tSettings::settingChanged, [=](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

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
