#!/usr/bin/env bash
set -euo pipefail

APP_NAME="klinkmonitor-native"
APP_TITLE="KLinkMonitor Native"

PREFIX="${HOME}/.local"

BIN_FILE="${PREFIX}/bin/${APP_NAME}"
DESKTOP_FILE="${PREFIX}/share/applications/${APP_NAME}.desktop"
ICON_FILE="${PREFIX}/share/icons/hicolor/scalable/apps/${APP_NAME}.svg"
AUTOSTART_FILE="${HOME}/.config/autostart/${APP_NAME}.desktop"

CONFIG_DIR="${HOME}/.config/${APP_NAME}"
DATA_DIR="${HOME}/.local/share/${APP_NAME}"

KILL="ask"
REMOVE_AUTOSTART="ask"
REMOVE_DATA="ask"
YES="no"

usage() {
  cat <<USAGE
Usage: ./uninstall.sh [options]

Options:
  --kill              Stop running ${APP_TITLE} processes
  --no-kill           Do not stop running processes
  --purge             Remove config and SQLite history
  --keep-data         Keep config and SQLite history
  -y, --yes           Use defaults for unanswered prompts
  --help, -h          Show this help
USAGE
}

ask_yes_no() {
  local prompt="$1" default="$2" answer

  if [[ "$YES" == "yes" ]]; then
    [[ "$default" == "yes" ]]
    return $?
  fi

  if [[ "$default" == "yes" ]]; then
    read -r -p "${prompt} [Y/n] " answer || true
    [[ -z "$answer" || "$answer" =~ ^[Yy]$ ]]
  else
    read -r -p "${prompt} [y/N] " answer || true
    [[ "$answer" =~ ^[Yy]$ ]]
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --kill) KILL="yes" ;;
    --no-kill) KILL="no" ;;
    --purge) REMOVE_DATA="yes" ;;
    --keep-data) REMOVE_DATA="no" ;;
    -y|--yes) YES="yes" ;;
    --help|-h) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
  shift
done

echo "Uninstalling ${APP_TITLE}"
echo

if [[ "$KILL" == "ask" ]]; then
  if ask_yes_no "Stop running ${APP_TITLE} processes?" "yes"; then
    KILL="yes"
  else
    KILL="no"
  fi
fi

if [[ "$REMOVE_AUTOSTART" == "ask" ]]; then
  if ask_yes_no "Remove KDE autostart entry?" "yes"; then
    REMOVE_AUTOSTART="yes"
  else
    REMOVE_AUTOSTART="no"
  fi
fi

if [[ "$REMOVE_DATA" == "ask" ]]; then
  if ask_yes_no "Remove configuration and SQLite history?" "no"; then
    REMOVE_DATA="yes"
  else
    REMOVE_DATA="no"
  fi
fi

if [[ "$KILL" == "yes" ]]; then
  pkill -x "$APP_NAME" 2>/dev/null || true
  pkill -f "/klinkmonitor-native" 2>/dev/null || true
  pkill -f "${HOME}/.local/bin/klinkmonitor-native" 2>/dev/null || true
fi

rm -f "$BIN_FILE"
rm -f "$DESKTOP_FILE"
rm -f "$ICON_FILE"

if [[ "$REMOVE_AUTOSTART" == "yes" ]]; then
  rm -f "$AUTOSTART_FILE"
fi

if [[ "$REMOVE_DATA" == "yes" ]]; then
  rm -rf "$CONFIG_DIR"
  rm -rf "$DATA_DIR"
fi

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${PREFIX}/share/applications" >/dev/null 2>&1 || true
fi

echo
echo "${APP_TITLE} removed successfully"
echo
echo "✔ Program files ....... Removed"
if [[ "$REMOVE_AUTOSTART" == "yes" ]]; then
  echo "✔ Autostart ........... Removed"
else
  echo "• Autostart ........... Preserved"
fi
if [[ "$REMOVE_DATA" == "yes" ]]; then
  echo "✔ Config/history ...... Removed"
else
  echo "• Config/history ...... Preserved"
fi
