#!/usr/bin/env bash
set -euo pipefail

APP_NAME="klinkmonitor-native"
APP_TITLE="KLinkMonitor Native"
APP_SRC_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
VERSION="$(cat "${APP_SRC_DIR}/VERSION" 2>/dev/null || echo "dev")"

PREFIX="${HOME}/.local"
BUILD_DIR="${APP_SRC_DIR}/build"
BIN_FILE="${PREFIX}/bin/${APP_NAME}"

START_NOW="yes"
YES="no"

usage() {
  cat <<USAGE
Usage: ./upgrade.sh [options]

Options:
  --start-now       Start ${APP_TITLE} after upgrade (default)
  --no-start        Do not start ${APP_TITLE} after upgrade
  --no-start-now    Alias for --no-start
  -y, --yes         Use defaults for unanswered prompts
  --help, -h        Show this help

Upgrades program files only. Configuration and SQLite history are preserved.
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
    --start-now) START_NOW="yes" ;;
    --no-start|--no-start-now) START_NOW="no" ;;
    -y|--yes) YES="yes" ;;
    --help|-h) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
  shift
done

echo "Upgrading ${APP_TITLE} to ${VERSION}"
echo

if [[ ! -x "$BIN_FILE" ]]; then
  echo "No existing installation found at: $BIN_FILE" >&2
  echo "Run ./install.sh instead." >&2
  exit 1
fi

echo "Configuring build..."
cmake -S "$APP_SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PREFIX"

echo
echo "Building..."
cmake --build "$BUILD_DIR"

echo
echo "Build completed successfully."

if [[ "$START_NOW" == "yes" ]]; then
  if ! ask_yes_no "Start ${APP_TITLE} after upgrade?" "yes"; then
    START_NOW="no"
  fi
fi

echo
echo "Installing program files..."

pkill -x "$APP_NAME" 2>/dev/null || true

cmake --install "$BUILD_DIR"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${PREFIX}/share/applications" >/dev/null 2>&1 || true
fi

if [[ "$START_NOW" == "yes" ]]; then
  nohup "$BIN_FILE" >/dev/null 2>&1 < /dev/null &
  disown || true
fi

echo
echo "${APP_TITLE} upgraded successfully"
echo
echo "✔ Build ............... OK"
echo "✔ Program files ....... Updated"
echo "✔ User data ........... Preserved"
if [[ "$START_NOW" == "yes" ]]; then
  echo "✔ Started ............. Yes"
else
  echo "• Started ............. No"
fi
