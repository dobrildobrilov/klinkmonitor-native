# KLinkMonitor Native

KLinkMonitor Native is the native C++ / Qt6 / KDE Frameworks implementation of KLinkMonitor for KDE Plasma.

## Features

- Native KDE StatusNotifierItem tray integration.
- Ethernet monitoring (1 Gbps → 100 Mbps drops and link-down events).
- Wi-Fi monitoring without notification spam from normal signal/bitrate drift.
- WireGuard, OpenVPN, TUN and virtual interface monitoring.
- SQLite history stored in `~/.local/share/klinkmonitor-native/history.sqlite3`.
- History, Statistics and Diagnostics dialogs.
- KDE autostart support.
- Single-instance mode.

## Dependencies on Arch Linux

```bash
sudo pacman -S cmake extra-cmake-modules qt6-base \
    kstatusnotifieritem kcoreaddons ki18n knotifications
```

## Install

```bash
./install.sh
```

By default, `install.sh` enables KDE autostart and starts KLinkMonitor Native immediately.

Useful options:

```bash
./install.sh --no-start
./install.sh --no-autostart
./install.sh --autostart --start-now -y
```

## Upgrade

Use this when KLinkMonitor Native is already installed and you want to replace only the program files. Configuration and SQLite history are preserved.

```bash
./upgrade.sh
```

Without immediate start:

```bash
./upgrade.sh --no-start
```

## Uninstall

```bash
./uninstall.sh
```

Remove application, configuration and SQLite history:

```bash
./uninstall.sh --purge
```

## Manual run

After installation:

```bash
~/.local/bin/klinkmonitor-native
```

Version:

```bash
~/.local/bin/klinkmonitor-native --version
```

## License

GPL-3.0-or-later
