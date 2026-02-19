#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env
r36s_require_host || exit $?

line_blue
info "Connecting to R36S Device"
line_blue

info_box "Host: $R36S_USER@$R36S_HOST"
info "Control Socket: $R36S_CONTROL_SOCKET"
info "This connection will persist for 10 minutes"
line_blue

# Reachability
if ! r36s_reachable_ssh; then
  line_red
  error "Cannot reach R36S $R36S_HOST:22 (SSH)"
  warn "Potential causes:"
  warn "  - R36S is powered off"
  warn "  - WiFi not connected on R36S"
  warn "  - Wrong IP Address in config/config.mk current setting $R36S_HOST"
  warn "  - R36S is not on same network (Host WiFi | Docker bridge | VPN | firewall)"
  warn "  - SSH is disabled on the R36S, run ./scripts/config_sd_card.sh"
  error "Fix one or all of the above and try again"
  line_red
  exit 3
fi

# Existing connection?
if r36s_socket_exists; then
  if r36s_socket_active; then
    success "Already connected to R36S"
    info "Connection is active and ready to use"
    exit 0
  else
    warn "Removing stale control socket"
    r36s_remove_socket
  fi
fi

info_box "Enter R36S password when prompted"

# Establish connection
SSH_ERR="$(mktemp)"
set +e
ssh -M -N -f \
  -o ControlMaster=yes \
  -o ControlPath="$R36S_CONTROL_SOCKET" \
  -o ControlPersist=10m \
  -o ConnectTimeout=5 \
  -o StrictHostKeyChecking=no \
  -o UserKnownHostsFile=/dev/null \
  "$R36S_USER@$R36S_HOST" 2>"$SSH_ERR"
SSH_RC=$?
set -e

if [[ $SSH_RC -ne 0 ]]; then
  RAW_ERR="$(tr -d '\r' <"$SSH_ERR" | tail -n 1)"
  rm -f "$SSH_ERR"

  line_red
  error "SSH failed: ${RAW_ERR:-unknown error}"
  warn "Potential causes:"
  warn "  - R36S is powered off"
  warn "  - WiFi not connected on R36S"
  warn "  - Wrong IP Address in config/config.mk current setting $R36S_HOST"
  warn "  - R36S is not on same network (Host WiFi | Docker bridge | VPN | firewall)"
  warn "  - SSH is disabled on the R36S, run ./scripts/config_sd_card.sh"
  error "Fix one or all of the above and try again"
  line_red

  r36s_remove_socket
  exit 1
fi
rm -f "$SSH_ERR"

sleep 1

# Verify
if r36s_socket_exists && r36s_socket_active; then
  success "Connected to R36S"
  info "Socket created: $R36S_CONTROL_SOCKET"
  info "Deploy binary and assets without re-entering password"
  info "Connection auto-expires in 10 minutes"
  info "Change ControlPersist=10m in ./scripts/ssh_connect_r36s.sh"
  info "To increase connection time e.g ControlPersist=20m"
  exit 0
else
  error "Failed to establish SSH control socket."
  r36s_remove_socket
  exit 1
fi