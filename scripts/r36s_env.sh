#!/usr/bin/env bash

#=================================================================
# Shared helper for R36S:
# - resolve REPO_ROOT
# - load messages
# - load config/config.mk
#=================================================================

set -euo pipefail

#=================================================================
# Resolve repo root
#=================================================================
r36s_resolve_paths() {
  local script_dir
  script_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
  REPO_ROOT="$(cd "$script_dir/.." && pwd)"
  export REPO_ROOT
}

#=================================================================
# Load message helpers
#=================================================================
r36s_load_messages() {
  # shellcheck source=/dev/null
  source "$REPO_ROOT/resources/messages.sh"
}

#=================================================================
# Load make-style config/config.mk into shell vars
# Supports: KEY ?= VALUE, KEY := VALUE, KEY = VALUE
# Ignores: make expansions like $(shell ...), $(PWD), etc.
#=================================================================
r36s_load_config_mk() {
  local file="$1"
  [[ -f "$file" ]] || return 1

  while IFS= read -r line; do
    line="${line%%#*}"
    line="$(printf '%s' "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
    [[ -z "$line" ]] && continue
    [[ "$line" == *'$('* ]] && continue

    if [[ "$line" =~ ^([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*(\?=|:=|=)[[:space:]]*(.*)$ ]]; then
      local key="${BASH_REMATCH[1]}"
      local op="${BASH_REMATCH[2]}"
      local val="${BASH_REMATCH[3]}"

      val="$(printf '%s' "$val" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
      val="${val%\"}"; val="${val#\"}"
      val="${val%\'}"; val="${val#\'}"

      # Only import vars we care about in scripts.
      case "$key" in
        R36S_USER|R36S_HOST|R36S_CONTROL_SOCKET|PING_TIMEOUT|R36S_PATH|R36S_GDB_PORT|R36S_SSH_KEY_PRIV|R36S_SSH_KEY_PUB)
          if [[ "$op" == "?=" ]]; then
            [[ -z "${!key:-}" ]] && printf -v "$key" '%s' "$val"
          else
            printf -v "$key" '%s' "$val"
          fi
          ;;
      esac
    fi
  done < "$file"
}

#=================================================================
# Public: load env (defaults + config.mk)
# Env vars passed in (from make) over config ?=
#=================================================================
r36s_load_env() {
  r36s_resolve_paths
  r36s_load_messages

  local config_mk="${REPO_ROOT}/config/config.mk"

  # Defaults (can be overridden by env or config.mk)
  R36S_USER="${R36S_USER:-}"
  R36S_HOST="${R36S_HOST:-}"
  R36S_CONTROL_SOCKET="${R36S_CONTROL_SOCKET:-/tmp/ssh-r36s-control}"
  PING_TIMEOUT="${PING_TIMEOUT:-2}"

  # Optional defaults if you later need them in scripts
  R36S_PATH="${R36S_PATH:-}"
  R36S_GDB_PORT="${R36S_GDB_PORT:-}"

  # SSH key paths (optional, only if using explicit keys)
  R36S_SSH_KEY_PRIV="${R36S_SSH_KEY_PRIV:-}"
  R36S_SSH_KEY_PUB="${R36S_SSH_KEY_PUB:-}"

  r36s_load_config_mk "$config_mk" || warn "Could not read $config_mk (using env/defaults)."

  # Final fallbacks
  R36S_USER="${R36S_USER:-ark}"

  export R36S_USER \
          R36S_HOST \
          R36S_CONTROL_SOCKET \
          PING_TIMEOUT \
          R36S_PATH \
          R36S_GDB_PORT \
          R36S_SSH_KEY_PRIV \
          R36S_SSH_KEY_PUB
}

#=================================================================
# Validation helpers
#=================================================================
r36s_require_host() {
  if [[ -z "${R36S_HOST:-}" ]]; then
    error "R36S_HOST is not set."
    warn "Fix one of these:"
    warn "  - Edit config/config.mk: R36S_HOST ?= 192.168.x.y"
    warn "  - Or run: R36S_HOST=192.168.x.y bash scripts/ssh_connect_r36s.sh"
    return 2
  fi
}

#=================================================================
# SSH control socket helpers
#=================================================================
r36s_socket_exists() {
  [[ -S "${R36S_CONTROL_SOCKET:-/tmp/ssh-r36s-control}" ]]
}

r36s_socket_active() {
  ssh -O check -S "$R36S_CONTROL_SOCKET" "$R36S_USER@$R36S_HOST" >/dev/null 2>&1
}

r36s_remove_socket() {
  rm -f "$R36S_CONTROL_SOCKET" >/dev/null 2>&1 || true
}

#=================================================================
# Require active SSH control socket (pre-target contract)
#=================================================================
r36s_require_connection() {
  if ! r36s_socket_exists; then
    line_red
    error "Missing SSH control socket: $R36S_CONTROL_SOCKET"
    warn "This script must be invoked with r36s_ssh_connection_up as a pre-target"
    line_red
    return 10
  fi

  if ! r36s_socket_active; then
    line_red
    error "SSH control socket is not active: $R36S_CONTROL_SOCKET"
    warn "This script must be invoked with r36s_ssh_connection_up as a pre-target"
    line_red
    return 10
  fi
}

#=================================================================
# Reachability check (nc is installed)
#=================================================================
r36s_reachable_ssh() {
  local timeout="${PING_TIMEOUT:-2}"
  nc -z -w "$timeout" "$R36S_HOST" 22 >/dev/null 2>&1
}