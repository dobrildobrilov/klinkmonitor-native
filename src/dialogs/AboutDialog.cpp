#include "AboutDialog.h"
#include "Version.h"
#include <QMessageBox>
#include <QApplication>
#include <QtGlobal>

namespace KLinkMonitorNative {
void showAboutDialog(QWidget *parent)
{
    QMessageBox::about(parent, QStringLiteral("About KLinkMonitor Native"),
        QStringLiteral("<h3>KLinkMonitor Native</h3>"
                       "<p>Version %1</p>"
                       "<p>Native Qt6/KDE Frameworks network link monitor.</p>"
                       "<p>Qt %2</p>"
                       "<p>License: GPL-3.0-or-later</p>")
            .arg(QStringLiteral(KLINKMONITOR_VERSION), QString::fromLatin1(qVersion())));
}
}
