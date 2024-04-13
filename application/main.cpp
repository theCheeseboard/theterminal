#include <QCommandLineParser>
#include <QJsonArray>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <abstractpty.h>
#include <tapplication.h>
#include <theterminal-init.h>
#include <tlogger.h>
#include <tsettings.h>
#include <tstylemanager.h>

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);
    a.setApplicationShareDir("theterminal");
    a.installTranslators();

    theTerminal::init();

    a.setApplicationVersion("1.0");
    a.setGenericName(QApplication::translate("main", "Terminal"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2024");
    a.setOrganizationName("theCheeseboard");
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

#if defined(Q_OS_MAC)
    a.setQuitOnLastWindowClosed(false);
#endif

    tSettings settings;
    QObject::connect(&settings, &tSettings::settingChanged, [=](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

    QQuickStyle::setStyle("com.vicr123.Contemporary.CoreStyles");
    QIcon::setThemeName("contemporary");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/com/vicr123/theterminal/Main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &a, [](QUrl url) {
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    QObject::connect(
        &engine, &QQmlApplicationEngine::warnings, &a, [](const QList<QQmlError>& warnings) {

    }, Qt::QueuedConnection);
    engine.load(url);

    return a.exec();
}
