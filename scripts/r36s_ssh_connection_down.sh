#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env
r36s_require_host || exit $?

info "Closing SSH connection to $R36S_USER@$R36S_HOST"

if r36s_socket_exists; then
  if r36s_socket_active; then
    ssh -O exit -S "$R36S_CONTROL_SOCKET" "$R36S_USER@$R36S_HOST" 2>/dev/null || true
    success "SSH connection closed"
  else
    warn "Stale control socket found (no active SSH master)"
  fi

  r36s_remove_socket
  info "Control socket removed"
else
  warn "No active SSH connection found"
fi