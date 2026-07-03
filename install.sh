#!/usr/bin/env bash
set -euo pipefail

APP_NAME="klinkmonitor-native"
APP_TITLE="KLinkMonitor Native"
APP_SRC_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
VERSION="$(cat "${APP_SRC_DIR}/VERSION" 2>/dev/null || echo "dev")"

PREFIX="${HOME}/.local"
BUILD_DIR="${APP_SRC_DIR}/build"

BIN_FILE="${PREFIX}/bin/${APP_NAME}"
DESKTOP_FILE="${PREFIX}/share/applications/${APP_NAME}.desktop"
AUTOSTART_DIR="${HOME}/.config/autostart"
AUTOSTART_FILE="${AUTOSTART_DIR}/${APP_NAME}.desktop"

YES="no"
AUTOSTART="ask"
START_NOW="yes"

usage() {
  cat <<USAGE
Usage: ./install.sh [options]

Options:
  --autostart       Enable KDE autostart without asking
  --no-autostart    Disable KDE autostart without asking
  --start-now       Start ${APP_TITLE} after installation (default)
  --no-start        Do not start ${APP_TITLE} after installation
  --no-start-now    Alias for --no-start
  -y, --yes         Use defaults for unanswered prompts
  --help, -h        Show this help

Installs only for the current user.
Install prefix: ${PREFIX}
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

check_cmd() {
  command -v "$1" >/dev/null 2>&1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --autostart) AUTOSTART="yes" ;;
    --no-autostart) AUTOSTART="no" ;;
    --start-now) START_NOW="yes" ;;
    --no-start|--no-start-now) START_NOW="no" ;;
    -y|--yes) YES="yes" ;;
    --help|-h) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
  shift
done

echo "${APP_TITLE} ${VERSION}"
echo

echo "Checking build environment..."

missing=()
check_cmd cmake || missing+=("cmake")
check_cmd c++ || missing+=("C++ compiler")

if ((${#missing[@]})); then
  echo "Missing dependencies: ${missing[*]}" >&2
  echo
  echo "On Arch Linux, install the required packages with:" >&2
  echo "  sudo pacman -S cmake extra-cmake-modules qt6-base kstatusnotifieritem kcoreaddons ki18n knotifications" >&2
  exit 1
fi

echo "✔ cmake"
echo "✔ C++ compiler"

echo
echo "Configuring build..."
cmake -S "$APP_SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PREFIX"

echo
echo "Building..."
cmake --build "$BUILD_DIR"

echo
echo "Build completed successfully."
echo

if [[ -x "$BIN_FILE" ]]; then
  echo "Existing installation found: $BIN_FILE"
  if ! ask_yes_no "Replace installed program files?" "yes"; then
    echo "Install cancelled."
    exit 0
  fi
fi

if [[ "$AUTOSTART" == "ask" ]]; then
  if ask_yes_no "Enable autostart with KDE?" "yes"; then
    AUTOSTART="yes"
  else
    AUTOSTART="no"
  fi
fi

if [[ "$START_NOW" == "yes" ]]; then
  if ! ask_yes_no "Start ${APP_TITLE} now?" "yes"; then
    START_NOW="no"
  fi
fi

echo
echo "Installing program files..."

pkill -x "$APP_NAME" 2>/dev/null || true

cmake --install "$BUILD_DIR"

if [[ "$AUTOSTART" == "yes" ]]; then
  mkdir -p "$AUTOSTART_DIR"
  if [[ -f "$DESKTOP_FILE" ]]; then
    cp "$DESKTOP_FILE" "$AUTOSTART_FILE"
  else
    echo "Warning: desktop file not found: $DESKTOP_FILE" >&2
    AUTOSTART="no"
  fi
else
  rm -f "$AUTOSTART_FILE"
fi

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${PREFIX}/share/applications" >/dev/null 2>&1 || true
fi

if [[ "$START_NOW" == "yes" ]]; then
  nohup "$BIN_FILE" >/dev/null 2>&1 < /dev/null &
  disown || true
fi

echo
echo "${APP_TITLE} ${VERSION} installed successfully"
echo
echo "✔ Build ............... OK"
echo "✔ Program files ....... Installed"
echo "✔ Desktop entry ....... Installed"
if [[ "$AUTOSTART" == "yes" ]]; then
  echo "✔ Autostart ........... Enabled"
else
  echo "• Autostart ........... Disabled"
fi
if [[ "$START_NOW" == "yes" ]]; then
  echo "✔ Started ............. Yes"
else
  echo "• Started ............. No"
fi
echo
echo "Run manually: $BIN_FILE"
