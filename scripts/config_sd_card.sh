#!/usr/bin/env bash
set -euo pipefail

#=================================================================
# R36S ArkOS Offline SD Card Configuration Script
# Configures ArkOS SD card for R36S to do the following:
# 1. Boot to console mode (no EmulationStation GUI)
# 2. Auto-connect to WiFi via NetworkManager (SSID and Password required)
# 3. Enable SSH server (see advanced SSH setup below and security note)
# 4. Auto-start Game
# 5. Test game shell script, terminal start game (test-game.sh)
# TODO: Complete test for dArkOS
#=================================================================

#=================================================================
# Color output
#=================================================================
if [[ -t 1 ]]; then
  readonly RED='\033[0;31m'
  readonly GREEN='\033[0;32m'
  readonly YELLOW='\033[1;33m'
  readonly BLUE='\033[0;34m'
  readonly NC='\033[0m' # No Color
else
  readonly RED='' GREEN='' YELLOW='' BLUE='' NC=''
fi

#=================================================================
# Logging functions
#=================================================================
log_info() { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARNING]${NC} $*" >&2; }
log_error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }
log_step() { echo -e "${BLUE}[STEP $1]${NC} $2"; }

#=================================================================
# Error handling
#=================================================================
cleanup_on_error() {
  local exit_code=$?
  if [[ $exit_code -ne 0 ]]; then
    log_error "Script failed at line $1 with exit code $exit_code"
    log_error "Partial changes may have been made to the SD card"
    log_info "Check logs and consider restoring from backups if needed"
  fi
}
trap 'cleanup_on_error $LINENO' ERR

#=================================================================
# Usage
#=================================================================
show_usage() {
  cat <<EOF
Usage: sudo $0 [OPTIONS] <SSID> <PASSWORD> [BUILD_DIR] [GAME_EXE]

Configure an ArkOS SD card for R36S offline.

Arguments:
  SSID         WiFi network name
  PASSWORD     WiFi password (stored in plaintext: see security note)
  BUILD_DIR    Optional: host folder to copy into /home/ark/autostart/
  GAME_RELEASE Optional: release binary name (default: gpp_r36s)

Options:
  -b PATH      Override BOOT partition mount point
  -r PATH      Override ROOT partition mount point
  -e PATH      Override EASYROMS partition mount point
  -h           Show this help message

Examples:
  sudo bash $0 "MyWiFi" "MyPassword"
  sudo bash $0 "MyWiFi" "MyPassword" ~/Projects/r36s/build/r36s_release gpp_r36s
  sudo bash $0 -b /mnt/boot -r /mnt/root "MyWiFi" "MyPassword"
  sudo bash ./config_sd_card.sh -b /media/muddygames/BOOT/ -r /media/muddygames/root/ -e /media/muddygames/EASYROMS/  "MyWiFi" "MyPassword" ~/Projects/r36s/build/r36s_release gpp_r36s

Security Note:
  WiFi password will be stored in plaintext at:
  /etc/NetworkManager/system-connections/*.nmconnection (mode 600, root-only)

EOF
}

#=================================================================
# Argument parsing
#=================================================================
# Detect the real user (not root when using sudo)
REAL_USER="${SUDO_USER:-$USER}"
BOOT="/media/$REAL_USER/BOOT"
ROOT="/media/$REAL_USER/root"
EASYROMS="/media/$REAL_USER/EASYROMS"

while getopts "b:r:e:h" opt; do
  case $opt in
    b) BOOT="$OPTARG" ;;
    r) ROOT="$OPTARG" ;;
    e) EASYROMS="$OPTARG" ;;
    h) show_usage; exit 0 ;;
    *) show_usage; exit 1 ;;
  esac
done
shift $((OPTIND-1))

SSID="${1:-}"
PSK="${2:-}"
BUILD_DIR="${3:-}"
GAME_RELEASE="${4:-gpp_r36s}"

if [[ -z "$SSID" || -z "$PSK" ]]; then
  log_error "Missing required arguments: SSID and PASSWORD"
  echo
  show_usage
  exit 1
fi

#=================================================================
# Validation functions
#=================================================================
need_root() {
  if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root (use sudo)"
    exit 1
  fi
}

need_dir() {
  local dir="$1"
  
  # Normalize path: add trailing slash and remove it to resolve symlinks
  local normalized="${dir%/}/"
  
  # Use -e (exists) instead of -d to handle edge cases with mounted filesystems
  if [[ ! -e "$normalized" ]]; then
    log_error "Path not found: $dir"
    log_info "Ensure the SD card is properly mounted"
    exit 1
  fi
  
  if [[ ! -d "$normalized" ]]; then
    log_error "Path exists but is not a directory: $dir"
    exit 1
  fi
  
  # Try to list the directory to ensure it's accessible
  if ! ls "$normalized" > /dev/null 2>&1; then
    log_error "Directory exists but is not accessible: $dir"
    log_info "Check permissions or mount status"
    exit 1
  fi
}

need_file() {
  if [[ ! -f "$1" ]]; then
    log_error "File not found: $1"
    exit 1
  fi
}

validate_arkos_root() {
  local root="$1"
  
  # Check for ArkOS-specific files (multiple markers, only one needs to exist)
  local markers=(
    "$root/etc/emulationstation/es_systems.cfg"
    "$root/usr/local/bin/perfmax"
    "$root/usr/local/bin/oga_controls"
    "$root/usr/local/bin/es_systems.cfg"
  )
  
  local found=0
  for marker in "${markers[@]}"; do
    if [[ -f "$marker" || -d "$marker" ]]; then
      found=1
      log_info "Found ArkOS marker: $marker"
      break
    fi
  done
  
  if [[ $found -eq 0 ]]; then
    log_error "Does not appear to be a valid ArkOS root filesystem"
    log_error "None of the expected ArkOS files were found:"
    for marker in "${markers[@]}"; do
      log_error "  - $marker"
    done
    log_info "Double-check your mount points with 'lsblk'"
    exit 1
  fi
  
  log_info "ArkOS root filesystem validated"
}

validate_r36s_boot() {
  local boot="$1"
  
  # Check for boot configuration (either boot.ini or extlinux)
  if [[ -f "$boot/boot.ini" ]]; then
    log_info "Found boot.ini configuration"
  elif [[ -d "$boot/extlinux" ]]; then
    log_info "Found extlinux boot configuration"
    # Check for extlinux.conf
    if [[ ! -f "$boot/extlinux/extlinux.conf" ]]; then
      log_warn "extlinux directory exists but extlinux.conf not found"
    fi
  else
    log_error "No valid boot configuration found (boot.ini or extlinux/)"
    log_error "This may not be a valid R36S boot partition"
    exit 1
  fi
  
  # Check for kernel image
  if [[ -f "$boot/Image" ]] || compgen -G "$boot/vmlinuz*" > /dev/null; then
    log_info "Kernel image found"
  else
    log_warn "No kernel image found (Image or vmlinuz*)"
  fi
  
  # Optional: Check for R36S device tree
  if compgen -G "$boot/*r36s*.dtb" > /dev/null || 
     compgen -G "$boot/*rk3326*.dtb" > /dev/null ||
     compgen -G "$boot/*.dtb" > /dev/null; then
    log_info "Device tree files found"
  else
    log_warn "No device tree files found"
  fi
}

safe_backup() {
  local file="$1"
  local backup="${file}.bak"
  
  if [[ -f "$file" && ! -f "$backup" ]]; then
    cp -a "$file" "$backup"
    log_info "Created backup: $backup"
  elif [[ -f "$backup" ]]; then
    log_info "Backup already exists: $backup"
  fi
}

#=================================================================
# Main Script
#=================================================================

log_info "R36S ArkOS Offline SD Configuration"
echo

#=================================================================
# Prerun checks
#=================================================================
need_root

log_info "Validating mount points"
need_dir "$BOOT"
need_dir "$ROOT"
need_dir "$EASYROMS"

log_info "Validating ArkOS installation"
validate_arkos_root "$ROOT"
validate_r36s_boot "$BOOT"

echo
log_info "Configuration Summary:"
log_info "  BOOT:         $BOOT"
log_info "  ROOT:         $ROOT"
log_info "  EASYROMS:     $EASYROMS"
log_info "  SSID:         $SSID"
log_info "  GAME_RELEASE: $GAME_RELEASE"
log_info "  WiFi password length: ${#PSK} characters"
echo

#=================================================================
# Confirmation prompt
#=================================================================
read -p "Proceed with SD card modification? (yes/no): " -r
echo
if [[ ! $REPLY =~ ^[Yy]es$ ]]; then
  log_info "Operation cancelled"
  exit 0
fi

#=================================================================
# Force console mode
#=================================================================
log_step "1/6" "Configuring boot for console mode"

if [[ -f "$BOOT/boot.ini" ]]; then
  # boot.ini configuration
  INI="$BOOT/boot.ini"
  safe_backup "$INI"

  if grep -q "systemd\.unit=multi-user\.target" "$INI"; then
    log_info "boot.ini already contains multi-user.target"
  else
    # Create temporary file for safer editing
    TMP_INI=$(mktemp)
    
    # More robust sed that handles multiple bootargs lines
    awk '
      /^setenv[[:space:]]+bootargs[[:space:]]+".*"/ {
        if (!modified && $0 !~ /systemd\.unit=multi-user\.target/) {
          sub(/"[[:space:]]*$/, " systemd.unit=multi-user.target\"")
          modified=1
        }
      }
      {print}
    ' "$INI" > "$TMP_INI"
    
    # Verify the change was made
    if grep -q "systemd\.unit=multi-user\.target" "$TMP_INI"; then
      mv "$TMP_INI" "$INI"
      log_info "Successfully patched boot.ini"
    else
      rm "$TMP_INI"
      log_error "Failed to patch boot.ini automatically"
      log_error "Please manually add 'systemd.unit=multi-user.target' to bootargs"
      exit 1
    fi
  fi

elif [[ -d "$BOOT/extlinux" ]]; then
  # Extlinux configuration
  EXTLINUX_CONF="$BOOT/extlinux/extlinux.conf"
  
  if [[ ! -f "$EXTLINUX_CONF" ]]; then
    log_error "extlinux directory exists but extlinux.conf not found"
    exit 1
  fi
  
  safe_backup "$EXTLINUX_CONF"
  
  if grep -q "systemd\.unit=multi-user\.target" "$EXTLINUX_CONF"; then
    log_info "extlinux.conf already contains multi-user.target"
  else
    # Patch the APPEND line in extlinux.conf
    TMP_CONF=$(mktemp)
    
    awk '
      /^[[:space:]]*APPEND/ {
        if (!modified && $0 !~ /systemd\.unit=multi-user\.target/) {
          sub(/[[:space:]]*$/, " systemd.unit=multi-user.target")
          modified=1
        }
      }
      {print}
    ' "$EXTLINUX_CONF" > "$TMP_CONF"
    
    # Verify the change was made
    if grep -q "systemd\.unit=multi-user\.target" "$TMP_CONF"; then
      mv "$TMP_CONF" "$EXTLINUX_CONF"
      log_info "Successfully patched extlinux.conf"
    else
      rm "$TMP_CONF"
      log_error "Failed to patch extlinux.conf automatically"
      log_error "Please manually add 'systemd.unit=multi-user.target' to APPEND line"
      exit 1
    fi
  fi
else
  log_error "No boot configuration found to patch"
  exit 1
fi

#=================================================================
# Configure WiFi via NetworkManager
#=================================================================

log_step "2/6" "Configuring NetworkManager WiFi profile"

WANTS="$ROOT/etc/systemd/system/multi-user.target.wants"
mkdir -p "$WANTS"

#=================================================================
# Ensure NetworkManager is enabled
#=================================================================
if [[ -f "$ROOT/lib/systemd/system/NetworkManager.service" ]]; then
  ln -sf /lib/systemd/system/NetworkManager.service \
    "$WANTS/NetworkManager.service" 2>/dev/null || true
  log_info "Info: NetworkManager service enabled"
else
  log_warn "Warning: NetworkManager.service not found - WiFi may not work"
fi

NM_CONNS="$ROOT/etc/NetworkManager/system-connections"
mkdir -p "$NM_CONNS"

#=================================================================
# Sanitize SSID for filename (remove problematic characters)
#=================================================================
SAFE_NAME=$(echo "$SSID" | sed 's/[^a-zA-Z0-9._-]/_/g')
CONN_FILE="$NM_CONNS/${SAFE_NAME}.nmconnection"

#=================================================================
# Backup existing connection if present
#=================================================================
if [[ -f "$CONN_FILE" ]]; then
  safe_backup "$CONN_FILE"
fi

#=================================================================
# Create NetworkManager connection file
#=================================================================
cat > "$CONN_FILE" <<EOF
[connection]
id=$SSID
type=wifi
autoconnect=true
autoconnect-priority=100

[wifi]
mode=infrastructure
ssid=$SSID

[wifi-security]
key-mgmt=wpa-psk
psk=$PSK

[ipv4]
method=auto

[ipv6]
method=auto
EOF

#=================================================================
# Set proper permissions (critical for NetworkManager)
#=================================================================
chown root:root "$CONN_FILE"
chmod 600 "$CONN_FILE"
log_info "WiFi profile created: $CONN_FILE (mode 600)"
log_warn "Password stored in plaintext (root-only access)"

#=================================================================
# Enable SSH
# TODO : Copy keys and disable password access SSH Only
#=================================================================
log_step "3/6" "Enabling SSH server"

if [[ -f "$ROOT/lib/systemd/system/ssh.service" ]]; then
  ln -sf /lib/systemd/system/ssh.service "$WANTS/ssh.service"
  log_info "SSH service enabled"
elif [[ -f "$ROOT/lib/systemd/system/sshd.service" ]]; then
  ln -sf /lib/systemd/system/sshd.service "$WANTS/sshd.service"
  log_info "SSHD service enabled"
else
  log_warn "SSH service not found - remote access may not work"
fi

#=================================================================
# Install autostart game
#=================================================================
log_step "4/6" "Configuring game autostart"

AUTO_DIR="$ROOT/home/ark/autostart"
mkdir -p "$AUTO_DIR"

#=================================================================
# Copy build directory if provided
#=================================================================
if [[ -n "$BUILD_DIR" ]]; then
  if [[ -d "$BUILD_DIR" ]]; then
    log_info "Copying build files: $BUILD_DIR -> $AUTO_DIR"
    cp -r "$BUILD_DIR"/. "$AUTO_DIR"/
    chown -R 1000:1000 "$AUTO_DIR"
    chmod -R u+rwX "$AUTO_DIR"
    log_info "Build files copied successfully"
  else
    log_warn "BUILD_DIR not found: $BUILD_DIR (skipping)"
  fi
fi

#=================================================================
# Check for game executable
#=================================================================
if [[ -f "$AUTO_DIR/$GAME_RELEASE" ]]; then
  chmod +x "$AUTO_DIR/$GAME_RELEASE"
  log_info "Game release binary ready: $GAME_RELEASE"
else
  log_warn "Game release binary not found: $AUTO_DIR/$GAME_RELEASE"
  log_warn "Autostart will fail until you copy the release files"
fi

#=================================================================
# Install first-boot permission fix script
#=================================================================
FIX_PERMS="$ROOT/usr/local/bin/fix-autostart-perms.sh"

cat > "$FIX_PERMS" <<'EOF'
#!/bin/sh

# Fix ownership and permissions for autostart directory
chown -R ark:ark /home/ark/autostart
chmod -R u+rwX /home/ark/autostart

# Ensure game binary is executable
if [ -f /home/ark/autostart/GAME_RELEASE_PLACEHOLDER ]; then
    chmod +x /home/ark/autostart/GAME_RELEASE_PLACEHOLDER
fi
EOF

sed -i "s/GAME_RELEASE_PLACEHOLDER/$GAME_RELEASE/g" "$FIX_PERMS"
chmod +x "$FIX_PERMS"
log_info "First-boot permission fix script created: $FIX_PERMS"

#=================================================================
# Create systemd service to run permission fix on first boot
#=================================================================
FIX_SVC="$ROOT/etc/systemd/system/fix-autostart-perms.service"

cat > "$FIX_SVC" <<EOF
[Unit]
Description=Fix autostart directory permissions
Before=autostart-game.service

[Service]
Type=oneshot
ExecStart=/usr/local/bin/fix-autostart-perms.sh

[Install]
WantedBy=multi-user.target
EOF

# Enable the fix-permissions service
ln -sf /etc/systemd/system/fix-autostart-perms.service \
  "$WANTS/fix-autostart-perms.service"

log_info "First-boot permission fix service enabled"

#=================================================================
# Autostart Script
#=================================================================
AUTO_SH="$ROOT/usr/local/bin/autostart-game.sh"
mkdir -p "$(dirname "$AUTO_SH")"

cat > "$AUTO_SH" <<'EOF'
#!/bin/sh

# Small delay to ensure DRM/KMS is ready
sleep 2

LOG="/home/ark/autostart.log"
echo "Autostart $(date)" >> "$LOG"

{
  echo "======================================"
  echo "Game Autostart Debug Log"
  echo "======================================"
  echo "Date: $(date)"
  echo "User: $(whoami)"
  echo "UID: $(id -u)"
  echo "Home: $HOME"
  echo "Working Dir: $(pwd)"
  echo "Path: $PATH"
  echo "IP Address:"
  hostname -I
  echo ""
  
  echo "WiFi interface:"
  ip addr show wlan0
  echo ""
  
  echo "Default route:"
  ip route show default
  echo ""
  
  echo "DNS test:"
  nslookup google.com 2>&1 || echo "DNS lookup failed"
  echo ""
  
  echo "Input devices:"
  ls -l /dev/input
  echo ""
  
  echo "DRM devices:"
  ls -l /dev/dri
  echo ""
  
  echo "Mounted filesystems:"
  mount | grep -E "boot|root"
  echo ""

  # Environment for Raylib/KMSDRM
  echo "Setting up environment"
  export HOME=/home/ark
  export USER=ark
  export SDL_VIDEODRIVER=kmsdrm
  export SDL_AUDIODRIVER=alsa

  echo "Environment variables set:"
  env | grep -E "HOME|USER|SDL"
  echo ""

  # Change to game directory
  echo "Changing to /home/ark/autostart"
  cd /home/ark/autostart || {
    echo "Error: Cannot cd to /home/ark/autostart/"
    exit 1
  }

  echo "Current directory: $(pwd)"
  echo "Directory contents:"
  ls -lah
  echo ""

  # Log IP address (ideally assigned by NetworkManager)
  # Also config.mk may set a IP for R36S
  # SSH connects to this IP Address
  echo "R36S_HOST=$(hostname -I | awk '{print $1}')"

  # Check for game binary
  echo "Looking for game binary: GAME_RELEASE_PLACEHOLDER"
  if [ ! -f "./GAME_RELEASE_PLACEHOLDER" ]; then
    echo "Error: GAME_RELEASE_PLACEHOLDER not found"
    echo "Files in directory:"
    find . -type f
    exit 1
  fi

  if [ ! -x "./GAME_RELEASE_PLACEHOLDER" ]; then
    echo "Error: GAME_RELEASE_PLACEHOLDER exists but is not executable"
    ls -l "./GAME_RELEASE_PLACEHOLDER"
    exit 1
  fi

  echo "Launching GAME_RELEASE_PLACEHOLDER at $(date)"
  ./GAME_RELEASE_PLACEHOLDER
  EXIT_CODE=$?

  echo "Game exited with code [$EXIT_CODE] at $(date)"
  exit $EXIT_CODE

} >> "$LOG" 2>&1
EOF

#=================================================================
# Replace GAME_RELEASE_PLACEHOLDER with Game binary name
#=================================================================
sed -i "s/GAME_RELEASE_PLACEHOLDER/$GAME_RELEASE/g" "$AUTO_SH"
chmod +x "$AUTO_SH"
log_info "Autostart script created: $AUTO_SH"

#=================================================================
# Create a autostart-game systemd service
#=================================================================
AUTO_SVC="$ROOT/etc/systemd/system/autostart-game.service"
cat > "$AUTO_SVC" <<EOF
[Unit]
Description=Autostart $GAME_RELEASE
After=multi-user.target

[Service]
Type=simple
User=ark
Group=ark
WorkingDirectory=/home/ark/autostart
ExecStart=/usr/local/bin/autostart-game.sh
Restart=no

# Critical: give DRM/KMS a real console
StandardInput=tty
StandardOutput=tty
TTYPath=/dev/tty1

[Install]
WantedBy=multi-user.target
EOF

#=================================================================
# Enable the service (offline symlink)
#=================================================================
ln -sf /etc/systemd/system/autostart-game.service \
  "$WANTS/autostart-game.service"
log_info "Autostart service enabled"

#=================================================================
# Disable getty on tty1 to prevent conflicts
#=================================================================
GETTY_LINK="$WANTS/getty@tty1.service"
if [[ -e "$GETTY_LINK" ]]; then
  rm -f "$GETTY_LINK"
  log_info "Disabled getty@tty1 to prevent TTY conflicts"
fi

#=================================================================
# Create a diagnostic service that runs early
#=================================================================
DIAG_SH="$ROOT/usr/local/bin/autostart-diagnostics.sh"
cat > "$DIAG_SH" <<'EOF'
#!/bin/bash
LOG="/boot/diagnostic.log"
{
  echo "======================================"
  echo "Pre-Boot Diagnostics"
  echo "Date: $(date)"
  echo "======================================"
  echo ""
  
  echo "System Info:"
  uname -a
  echo ""
  
  echo "Systemd targets:"
  systemctl list-units --type=target --state=active
  echo ""
  
  echo "Checking autostart service:"
  systemctl status autostart-game.service --no-pager --full || echo "Service status check failed"
  echo ""
  
  echo "Checking if service executable (startable):"
  systemctl is-enabled autostart-game.service || echo "Service not enabled"
  systemctl is-failed autostart-game.service || echo "Service status check"
  echo ""
  
  echo "Service journal (last 50 lines):"
  journalctl -u autostart-game.service -n 50 --no-pager || echo "No journal entries"
  echo ""
  
  echo "Journal entries for autostart-game:"
  journalctl -u autostart-game.service --all --no-pager || echo "No entries at all"
  echo ""
  
  echo "Checking for /boot/autostart.log:"
  if [ -f /boot/autostart.log ]; then
    echo "File exists with contents:"
    cat /boot/autostart.log
  else
    echo "Warning: File does NOT exist (service never ran)"
  fi
  echo ""
  
  echo "Checking for /boot/autostart.log:"
  if [ -f /boot/autostart.log ]; then
    echo "Fallback log exists [Contents]:"
    cat /boot/autostart.log
  else
    echo "Warning: Fallback log does not exist"
  fi
  echo ""
  
  echo "Checking /home/ark/autostart:"
  if [ -d /home/ark/autostart ]; then
    echo "Success: Directory exists"
    ls -lah /home/ark/autostart
  else
    echo "Error: Directory does NOT exist"
  fi
  echo ""
  
  echo "Checking script:"
  if [ -f /usr/local/bin/autostart-game.sh ]; then
    echo "Success: Script exists"
    ls -l /usr/local/bin/autostart-game.sh
    echo "Info: First 10 lines:"
    head -10 /usr/local/bin/autostart-game.sh
  else
    echo "Warning: Script does NOT exist"
  fi
  echo ""
  
  echo "Network status:"
  ip addr show
  echo ""
  
  echo "IP Addresses assigned:"
  hostname -I || echo "Warning: No IP addresses assigned"
  echo ""
  
  echo "Specific interface IPs:"
  ip -4 addr show wlan0 | grep inet || echo "No IPv4 on wlan0"
  ip -4 addr show eth0 | grep inet || echo "No IPv4 on eth0"
  echo ""
  
  echo "Default route:"
  ip route show default || echo "Warning: No default route"
  echo ""
  
  echo "NetworkManager status:"
  systemctl status NetworkManager --no-pager || echo "Warning: NetworkManager not running"
  echo ""
  
  echo "WiFi connections:"
  ls -la /etc/NetworkManager/system-connections/ || echo "Warning: No connections found"
  echo ""
  
  echo "NetworkManager connection status (if available):"
  nmcli device status 2>/dev/null || echo "Warning: nmcli not available"
  echo ""
  
  echo "Active connections:"
  nmcli connection show --active 2>/dev/null || echo "Warning: nmcli not available"
  echo ""
  
  echo "Mounted filesystems:"
  mount | grep -E "boot|root"
  echo ""
  
  echo "======================================"
} > "$LOG" 2>&1
EOF
chmod +x "$DIAG_SH"

#=================================================================
# Autostart-Diagnostics
#=================================================================
DIAG_SVC="$ROOT/etc/systemd/system/autostart-diagnostics.service"
cat > "$DIAG_SVC" <<EOF
[Unit]
Description=Autostart Diagnostics
Before=autostart-game.service
After=multi-user.target
After=network-online.target
Wants=network-online.target

[Service]
Type=oneshot
ExecStart=/bin/bash /usr/local/bin/autostart-diagnostics.sh
ExecStartPost=/bin/sleep 10
ExecStartPost=/bin/bash /usr/local/bin/autostart-diagnostics-delayed.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

ln -sf /etc/systemd/system/autostart-diagnostics.service \
  "$WANTS/autostart-diagnostics.service"
log_info "Diagnostic service enabled"

#=================================================================
# Create a delayed diagnostic script that runs 10 seconds later
#=================================================================
DIAG_DELAYED_SH="$ROOT/usr/local/bin/autostart-diagnostics-delayed.sh"
cat > "$DIAG_DELAYED_SH" <<'EOF'
#!/bin/bash
LOG="/boot/diagnostic-delayed.log"
{
  echo "======================================"
  echo "Delayed Diagnostics (10s after boot)"
  echo "Date: $(date)"
  echo "======================================"
  echo ""
  
  echo "IP Address:"
  hostname -I || echo "Warning: No IP address"
  echo ""
  
  echo "WiFi interface status:"
  ip addr show wlan0
  echo ""
  
  echo "WiFi connected:"
  nmcli device status 2>/dev/null || echo "Warning: nmcli not available"
  echo ""
  
  echo "Active NetworkManager connections:"
  nmcli connection show --active 2>/dev/null || echo "Warning: No active connections or nmcli unavailable"
  echo ""
  
  echo "Ping gateway:"
  GATEWAY=$(ip route show default | awk '/default/ {print $3}' | head -1)
  if [ -n "$GATEWAY" ]; then
    echo "Gateway: $GATEWAY"
    ping -c 3 "$GATEWAY" || echo "Warning: cannot ping gateway"
  else
    echo "Warning: No gateway found"
  fi
  echo ""
  
  echo "DNS resolution test:"
  nslookup google.com 2>&1 || echo "Warning: DNS resolution failed"
  echo ""
  
  echo "======================================"
} > "$LOG" 2>&1
EOF
chmod +x "$DIAG_DELAYED_SH"
log_info "Delayed diagnostic script created"

# Create a manual test script
TEST_SH="$ROOT/home/ark/test-game.sh"
cat > "$TEST_SH" <<'EOF'
#!/bin/bash
echo "#====================================="
echo "Manual Game Test Script"
echo "#====================================="
echo ""

cd /home/ark/autostart || {
  echo "Error: Cannot cd to /home/ark/autostart"
  exit 1
}

echo "Info: Current directory: $(pwd)"
echo "Info: Files present:"
ls -lah
echo ""

export HOME=/home/ark
export USER=ark
export SDL_VIDEODRIVER=kmsdrm
export SDL_AUDIODRIVER=alsa
export XDG_RUNTIME_DIR=/run/user/1000

echo "Info: Launching GAME_RELEASE_PLACEHOLDER"
./GAME_RELEASE_PLACEHOLDER
echo "Info: Game exited with code: $?"
EOF

#=================================================================
# Replace GAME_RELEASE_PLACEHOLDER with Game binary name
#=================================================================
sed -i "s/GAME_RELEASE_PLACEHOLDER/$GAME_RELEASE/g" "$TEST_SH"
chmod +x "$TEST_SH"
chown 1000:1000 "$TEST_SH" 2>/dev/null || true
log_info "Manual test script created: /home/ark/test-game.sh: $TEST_SH"

#=================================================================
# Disable EmulationStation
#=================================================================
log_step "5/6" "Disabling EmulationStation"

ES_LINK="$WANTS/emulationstation.service"

if [[ -L "$ES_LINK" ]] || [[ -f "$ES_LINK" ]]; then
  # Create backup before removing
  if [[ ! -f "$WANTS/emulationstation.service.disabled" ]]; then
    cp -a "$ES_LINK" "$WANTS/emulationstation.service.disabled" 2>/dev/null || true
    log_info "EmulationStation service backed up"
  fi
  
  rm -f "$ES_LINK"
  log_info "EmulationStation service disabled"
else
  log_info "EmulationStation not enabled (already disabled)"
fi

#=================================================================
# Write configuration markers and logs
#=================================================================
log_step "6/6" "Writing configuration"

#=================================================================
# Create detailed log
#=================================================================
CONFIG_LOG="$BOOT/offline-config.log"
cat >> "$CONFIG_LOG" <<EOF
=================================================================
Configuration Date: $(date -u +"%Y-%m-%d %H:%M:%S UTC")
-----------------------------------------------------------------
SSID: $SSID
Game Release: $GAME_RELEASE
Build Directory: ${BUILD_DIR:-"(none)"}
-----------------------------------------------------------------
Boot Partition: $BOOT
Root Partition: $ROOT
=================================================================

EOF

log_info "Configuration log: $CONFIG_LOG"

# Create success marker
touch "$BOOT/.offline-config-success"

#=================================================================
# Completion
#=================================================================
echo
log_info "${GREEN}Configuration completed successfully ${NC}"
echo
cat <<EOF
Next steps:
  1. Safely unmount the SD card:
     sudo umount "$BOOT" "$ROOT" "$EASYROMS"
  
  2. Insert SD card into R36S and power on

Expected behavior:
  [X] Boot to console (no EmulationStation)
  [X] Automatic WiFi connection to "$SSID"
  [X] SSH server running (e.g: ark@192.168.0.X)
  [X] Game autostart: $GAME_RELEASE

Logs available on BOOT partition:
  - /home/ark/autostart.log     (Game startup and output)
  - $BOOT/offline-config.log    (this configuration)
  - $BOOT/diagnostic.log        (startup diagnostics)
  - $BOOT/diagnostic-delayed.log(Wifi config etc)
  
Troubleshooting:
  - Default login: ark / ark
  - Check logs with: tail -f $BOOT/autostart.log
  - Disable autostart: sudo systemctl disable autostart-game
  - Re-enable EmulationStation: sudo systemctl enable emulationstation

To restore original boot configuration:
  Boot.ini: sudo cp $BOOT/boot.ini.bak $BOOT/boot.ini
  Extlinux: sudo cp $BOOT/extlinux/extlinux.conf.bak $BOOT/extlinux/extlinux.conf
EOF
echo