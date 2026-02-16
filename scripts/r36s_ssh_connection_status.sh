#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env
r36s_require_host || exit $?

info "Checking SSH control connection for $R36S_USER@$R36S_HOST"
info "Control Socket: $R36S_CONTROL_SOCKET"

if ! r36s_socket_exists; then
  error "No control socket found at $R36S_CONTROL_SOCKET"
  warn "Run: make -f Makefile.mk ./scripts/ssh_connect_r36s.sh"
  exit 3
fi

if r36s_socket_active; then
  success "SSH connection is active"
  exit 0
else
  error "Connection check failed (stale socket or host unreachable)"
  warn "Try:"
  warn "  - ./scripts/ssh_disconnect_r36s.sh (to remove stale socket)"
  warn "  - make -f Makefile.mk connect_r36s"
  exit 1
fi