#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>
#include <QString>

#include <KAboutData>
#include <KLocalizedString>

#include "Version.h"
#include "app/App.h"

static constexpr const char *APP_ID = "klinkmonitor-native";
static constexpr const char *INSTANCE_ID = "klinkmonitor-native.instance";

static bool alreadyRunning()
{
    QLocalSocket socket;
    socket.connectToServer(QString::fromLatin1(INSTANCE_ID));

    if (socket.waitForConnected(100)) {
        return true;
    }

    QLocalServer::removeServer(QString::fromLatin1(INSTANCE_ID));

    auto *server = new QLocalServer(qApp);
    server->listen(QString::fromLatin1(INSTANCE_ID));

    return false;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName(QStringLiteral("Dobril"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("github.com/dobrildobrilov"));

    QCoreApplication::setApplicationName(QStringLiteral("klinkmonitor-native"));

    QApplication::setApplicationDisplayName(QStringLiteral("KLinkMonitor Native"));
    QApplication::setApplicationVersion(QStringLiteral(KLINKMONITOR_VERSION));

    QGuiApplication::setDesktopFileName(QString::fromLatin1(APP_ID));

    KLocalizedString::setApplicationDomain("klinkmonitor-native");

    const QStringList args = application.arguments();

    if (args.contains(QStringLiteral("--version"))) {
        qInfo().noquote()
            << QStringLiteral("KLinkMonitor Native %1")
                   .arg(QStringLiteral(KLINKMONITOR_VERSION));
        return 0;
    }

    KAboutData aboutData(
        QString::fromLatin1(APP_ID),
        i18n("KLinkMonitor Native"),
        QStringLiteral(KLINKMONITOR_VERSION),
        i18n("Ethernet, Wi-Fi and VPN link monitor"),
        KAboutLicense::GPL_V3,
        i18n("Copyright © 2026 Dobril Dobrilov")
    );

    aboutData.addAuthor(
        i18n("Dobril Dobrilov"),
        i18n("Author and maintainer"),
        QString(),
        QStringLiteral("https://github.com/dobrildobrilov")
    );

    aboutData.setHomepage(
        QStringLiteral("https://github.com/dobrildobrilov/klinkmonitor-native")
    );

    aboutData.setDesktopFileName(QString::fromLatin1(APP_ID));

    KAboutData::setApplicationData(aboutData);

    if (alreadyRunning()) {
        qInfo().noquote() << QStringLiteral("KLinkMonitor Native is already running.");
        return 0;
    }

    KLinkMonitorNative::App app;

    return application.exec();
}
