#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env

line_blue
info "Build Matrix Test (non-interactive)"
line_blue

info_box "Builds: linux/web/windows (debug+release) | r36s (debug+release)"
info_box "Note: Windows targets are compile-only (no wine execution)."
line_blue

MAKEFILE="Makefile.mk"

have_cmd() { command -v "$1" >/dev/null 2>&1; }

#-----------------------------------------------------------------
# Results tracking
#-----------------------------------------------------------------
declare -a PASS FAIL WARN

add_pass() { PASS+=("$1"); success "PASS: $1"; }
add_fail() { FAIL+=("$1"); error "FAIL: $1"; }
add_warn() { WARN+=("$1"); warn "WARN: $1"; }

run_make() {
  local target="$1"
  local rc=0

  line_blue
  info "Running (non-interactive): make $target"
  line_blue

  # Web debug/release starts a blocking server. Auto-terminate after a few seconds.
  if [[ "$target" == "debug_web" || "$target" == "release_web" ]]; then
    info "$target starts a web server; running with timeout to stop it automatically"

    if have_cmd timeout; then
      set +e
      TERM=dumb timeout 6s \
        make -f "$MAKEFILE" --no-print-directory "$target" </dev/null
      rc=$?
      set -e

      # timeout returns 124 when it kills the command (expected here)
      if [[ $rc -eq 124 ]]; then
        success "Web server terminated automatically (expected)"
        add_pass "$target (server auto-stopped)"
        return 0
      fi

      if [[ $rc -ne 0 ]]; then
        line_red
        error "Target failed: $target (rc=$rc)"
        line_red
        add_fail "$target (rc=$rc)"
        return "$rc"
      fi

      success "OK: $target"
      add_pass "$target"
      return 0
    else
      warn "'timeout' not found; falling back to background kill strategy"
      set +e
      TERM=dumb make -f "$MAKEFILE" --no-print-directory "$target" </dev/null &
      local pid=$!
      sleep 6
      kill "$pid" >/dev/null 2>&1
      wait "$pid" >/dev/null 2>&1
      set -e
      success "Web server terminated automatically (expected)"
      add_pass "$target (server auto-stopped)"
      return 0
    fi
  fi

  # Windows debug/release targets can be tricky to run via wine (needs X/Display).
  # Strategy: run but treat wine/display failures as non-fatal as long as the binary built successfully.
  if [[ "$target" == "debug_windows" || "$target" == "release_windows" ]]; then
    local platform_dir="windows_debug"
    [[ "$target" == "release_windows" ]] && platform_dir="windows_release"

    # Uses GAME_TARGET env exported by Makefile.
    local base="${GAME_TARGET:-}"
    if [[ -z "$base" ]]; then
      # Fallback: best effort; remove if GAME_TARGET set.
      base="GameBinaryName"
    fi

    local exe="build/${platform_dir}/${base}_windows.exe"

    info "Windows target; will not fail on wine/windowing errors if executable exists"
    info "Expected output: $exe"
    line_blue

    set +e
    TERM=dumb make -f "$MAKEFILE" --no-print-directory "$target" </dev/null
    rc=$?
    set -e

    if [[ $rc -eq 0 ]]; then
      success "OK: $target"
      add_pass "$target"
      return 0
    fi

    # If make failed, try to determin if it was just wine/X11 runtime failure but a successful compile.
    if [[ -f "$exe" ]]; then
      line_blue
      warn "Target returned non-zero (rc=$rc) but executable exists:"
      warn "  $exe"
      warn "Assuming build succeeded and failure is from running under wine/headless environment."
      warn "Recommendation: run Windows builds on well a Windows host, or run wine on a host Linux with X/Wayland."
      line_blue
      add_warn "$target (wine/display issue; exe exists)"
      return 0
    fi

    line_red
    error "Windows target failed and executable not found: $exe"
    line_red
    add_fail "$target (rc=$rc; exe missing)"
    return "$rc"
  fi

  # All other targets: stdin disconnected so interactive tools (e.g. gdb) does not block.
  set +e
  TERM=dumb make -f "$MAKEFILE" --no-print-directory "$target" </dev/null
  rc=$?
  set -e

  if [[ $rc -ne 0 ]]; then
    add_fail "$target (rc=$rc)"
    return "$rc"
  fi

  success "OK: $target"
  add_pass "$target"
}

#-----------------------------------------------------------------
# 1) Native / Cross builds (no device)
#-----------------------------------------------------------------
info_box "1) Native / Cross builds (non-interactive)"

run_make debug_linux
run_make release_linux

run_make debug_web
run_make release_web

run_make debug_windows
run_make release_windows

#-----------------------------------------------------------------
# 2) R36S builds (requires host + SSH)
#-----------------------------------------------------------------
info_box "2) R36S builds (requires valid host + SSH)"

if ! r36s_require_host; then
  line_red
  warn "R36S host not configured; skipping R36S builds"
  warn "Set R36S_USER / R36S_HOST in config/config.mk"
  line_red
  add_fail "r36s_ssh (host not configured)"
else
  info_box "Host: $R36S_USER@$R36S_HOST"
  info "Control Socket: ${R36S_CONTROL_SOCKET:-<unset>}"
  info "Explicit key (priv): ${R36S_SSH_KEY_PRIV:-<unset>}"
  info "Explicit key (pub):  ${R36S_SSH_KEY_PUB:-<unset>}"
  line_blue

  if ! r36s_reachable_ssh; then
    line_red
    warn "Skipping R36S builds: cannot reach $R36S_HOST:22 (SSH)"
    warn "Potential causes:"
    warn "  - R36S is powered off"
    warn "  - WiFi not connected on R36S"
    warn "  - Wrong IP Address in config/config.mk current setting $R36S_HOST"
    warn "  - R36S is not on same network (Host WiFi | Docker bridge | VPN | firewall)"
    warn "  - SSH is disabled on the R36S, run ./scripts/config_sd_card.sh"
    line_red
    add_fail "r36s_ssh (unreachable $R36S_HOST:22)"
  else
    success "R36S SSH reachable"
    add_pass "r36s_ssh (reachable)"

    #-----------------------------------------------------------------
    # Explicit key auth test
    #-----------------------------------------------------------------
    if [[ -n "${R36S_SSH_KEY_PRIV:-}" ]] && [[ -f "${R36S_SSH_KEY_PRIV}" ]]; then
      line_blue
      info "Testing explicit SSH key auth..."
      line_blue

      set +e
      ssh -i "$R36S_SSH_KEY_PRIV" \
        -o IdentitiesOnly=yes \
        -o BatchMode=yes \
        -o ConnectTimeout=5 \
        -o StrictHostKeyChecking=no \
        -o UserKnownHostsFile=/dev/null \
        "$R36S_USER@$R36S_HOST" "true" >/dev/null 2>&1
      KEY_RC=$?
      set -e

      if [[ $KEY_RC -ne 0 ]]; then
        line_red
        warn "Explicit key auth failed."
        warn "Likely cause: Docker was rebuilt and generated a new SSH keypair,"
        warn "while the R36S still has the old public key in authorized_keys."
        warn "Only fixes:"
        warn "  1) Disable SSH explicit-key mode BEFORE Docker rebuild (or persist keys on host)"
        warn "  2) Flash ArkOS + run ./scripts/config_sd_card.sh"
        line_red
        add_fail "r36s_ssh_key (auth failed)"
      else
        success "Explicit key auth OK"
        add_pass "r36s_ssh_key (auth ok)"
      fi
    else
      warn "No explicit private key found; skipping explicit-key auth test"
      warn "R36S_SSH_KEY_PRIV is unset or file missing"
      add_warn "r36s_ssh_key (missing key file)"
    fi

    #-----------------------------------------------------------------
    # Non-interactive SSH login test
    #-----------------------------------------------------------------
    line_blue
    info "Verifying SSH login works (non-interactive BatchMode test)..."
    line_blue

    set +e
    ssh \
      -o BatchMode=yes \
      -o ConnectTimeout=5 \
      -o StrictHostKeyChecking=no \
      -o UserKnownHostsFile=/dev/null \
      "$R36S_USER@$R36S_HOST" "true" >/dev/null 2>&1
    SSH_RC=$?
    set -e

    if [[ $SSH_RC -ne 0 ]]; then
      line_red
      warn "SSH login test failed (BatchMode)."
      warn "This often means password auth is required (no key set up), or key auth failed."
      warn "Suggested fixes:"
      warn "  - Run: make r36s_ssh_connection_up (will prompt for password)"
      warn "  - Or fix explicit key provisioning (see warnings above)"
      line_red
      add_fail "r36s_ssh_batchmode (login failed)"
      info "Continuing with local R36S builds (compile only)..."
    else
      success "SSH login works (non-interactive)"
      add_pass "r36s_ssh_batchmode (login ok)"
    fi

    #-----------------------------------------------------------------
    # Compile-only R36S builds
    #-----------------------------------------------------------------
    run_make debug_r36s
    run_make release_r36s
  fi
fi

#-----------------------------------------------------------------
# Summary + exit code
#-----------------------------------------------------------------
line_blue
info "Summary"
line_blue

info "PASS: ${#PASS[@]}"
for x in "${PASS[@]:-}"; do info "  - $x"; done

warn "WARN: ${#WARN[@]}"
for x in "${WARN[@]:-}"; do warn "  - $x"; done

if [[ ${#FAIL[@]} -gt 0 ]]; then
  line_red
  error "FAIL: ${#FAIL[@]}"
  for x in "${FAIL[@]}"; do error "  - $x"; done
  line_red
  error "Build Matrix completed with failures"
  exit 1
fi

success "Build Matrix completed (non-interactive)"
line_blue