#include "SettingsDialog.h"
#include "settings/Settings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace KLinkMonitorNative {
SettingsDialog::SettingsDialog(Settings &settings, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("KLinkMonitor Native Settings"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    auto *interval = new QSpinBox(this);
    interval->setRange(1, 60);
    interval->setValue(settings.intervalMs() / 1000);
    interval->setSuffix(QStringLiteral(" sec"));
    auto *notifications = new QCheckBox(QStringLiteral("Enable notifications"), this);
    notifications->setChecked(settings.notificationsEnabled());
    form->addRow(QStringLiteral("Check interval"), interval);
    form->addRow(QString(), notifications);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this, &settings, interval, notifications]() {
        settings.setIntervalMs(interval->value() * 1000);
        settings.setNotificationsEnabled(notifications->isChecked());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
}
