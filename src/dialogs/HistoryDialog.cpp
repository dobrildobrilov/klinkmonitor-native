#include "HistoryDialog.h"
#include "database/HistoryDb.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace KLinkMonitorNative {

HistoryDialog::HistoryDialog(HistoryDb &history, const QString &interfaceName, QWidget *parent)
    : QDialog(parent), m_history(history), m_interfaceName(interfaceName)
{
    setWindowTitle(QStringLiteral("KLinkMonitor Native Event Log"));
    resize(820, 420);

    auto *layout = new QVBoxLayout(this);
    auto *table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("Time"), QStringLiteral("Interface"), QStringLiteral("Status"), QStringLiteral("Event"), QStringLiteral("Change")});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    const auto rows = m_history.events(interfaceName, 500);
    table->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const auto &r = rows.at(i);
        QString status = QStringLiteral("🟢");
        if (r.severity == QStringLiteral("warning")) status = QStringLiteral("🟡");
        if (r.severity == QStringLiteral("critical")) status = QStringLiteral("🔴");
        const QString change = r.oldValue.isEmpty() ? r.newValue : QStringLiteral("%1 → %2").arg(r.oldValue, r.newValue);
        table->setItem(i, 0, new QTableWidgetItem(r.timestamp));
        table->setItem(i, 1, new QTableWidgetItem(r.interfaceName));
        table->setItem(i, 2, new QTableWidgetItem(status));
        table->setItem(i, 3, new QTableWidgetItem(eventLabel(r.event, r.severity)));
        table->setItem(i, 4, new QTableWidgetItem(change));
    }
    layout->addWidget(table);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *clear = buttons->addButton(QStringLiteral("Clear Event Log"), QDialogButtonBox::DestructiveRole);
    connect(clear, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, QStringLiteral("Clear Event Log"), QStringLiteral("Clear all logged events?")) == QMessageBox::Yes) {
            m_history.clearEvents();
            accept();
        }
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString HistoryDialog::eventLabel(const QString &event, const QString &) const
{
    return event;
}

}
