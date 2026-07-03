#pragma once

#include <QDialog>

namespace KLinkMonitorNative {

class Settings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings &settings, QWidget *parent = nullptr);
};

} // namespace KLinkMonitorNative
