#include "StatisticsDialog.h"
#include "database/HistoryDb.h"
#include "monitor/InterfaceMonitor.h"

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace KLinkMonitorNative {

static QString healthLabel(int totalProblems)
{
    if (totalProblems == 0) return QStringLiteral("Excellent (100%)");
    if (totalProblems <= 2) return QStringLiteral("Good (90%)");
    if (totalProblems <= 8) return QStringLiteral("Fair (75%)");
    return QStringLiteral("Poor (50%)");
}

StatisticsDialog::StatisticsDialog(HistoryDb &history, InterfaceMonitor &monitor, const QString &interfaceName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("KLinkMonitor Native Statistics"));
    resize(720, 480);
    auto *layout = new QVBoxLayout(this);
    auto *text = new QPlainTextEdit(this);
    text->setReadOnly(true);

    const QString scope = interfaceName.isEmpty() ? QStringLiteral("All interfaces") : interfaceName;
    const auto info = interfaceName.isEmpty() ? InterfaceInfo{} : monitor.readInfo(interfaceName);
    const int dropsToday = history.countSpeedDrops(interfaceName, 1);
    const int drops7 = history.countSpeedDrops(interfaceName, 7);
    const int dropsTotal = history.countSpeedDrops(interfaceName, -1);
    const int downToday = history.countLinkDown(interfaceName, 1);
    const int down7 = history.countLinkDown(interfaceName, 7);
    const int downTotal = history.countLinkDown(interfaceName, -1);
    const int problemsTotal = history.countProblems(interfaceName, -1);

    QString diagnostics;
    if (problemsTotal == 0) {
        diagnostics = QStringLiteral("No problems recorded for the selected scope.\nCurrent health looks stable.");
    } else {
        diagnostics = QStringLiteral("Problems recorded for the selected scope:\n");
        if (dropsTotal > 0) diagnostics += QStringLiteral("- Ethernet speed drops detected: %1\n").arg(dropsTotal);
        if (downTotal > 0) diagnostics += QStringLiteral("- Link down events detected: %1\n").arg(downTotal);
    }

    text->setPlainText(QStringLiteral(
        "Scope: %1\n\n"
        "Current\n-------\n"
        "Current value: %2\n"
        "Current state: %3\n\n"
        "Speed drops (1G → 100M)\n------------------------\n"
        "Today: %4\nLast 7 days: %5\nTotal: %6\n\n"
        "Link Down events\n----------------\n"
        "Today: %7\nLast 7 days: %8\nTotal: %9\n\n"
        "Monitoring\n----------\n"
        "First event: %10\nLast problem: %11\n\n"
        "Health\n------\n%12\n\n"
        "Diagnostics\n-----------\n%13")
        .arg(scope,
             interfaceName.isEmpty() ? QStringLiteral("N/A") : info.displayValue,
             interfaceName.isEmpty() ? QStringLiteral("N/A") : InterfaceMonitor::stateIcon(info.state),
             QString::number(dropsToday), QString::number(drops7), QString::number(dropsTotal),
             QString::number(downToday), QString::number(down7), QString::number(downTotal),
             history.firstSeen(interfaceName), history.lastProblem(interfaceName),
             healthLabel(problemsTotal), diagnostics));

    layout->addWidget(text);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
}
