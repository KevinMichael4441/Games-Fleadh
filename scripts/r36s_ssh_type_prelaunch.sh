#!/usr/bin/env bash
set -euo pipefail

MODE="${R36S_DEPLOY_MODE:-ssh-default}"

case "$MODE" in
  ssh-default)
    echo "[R36S] PreLaunch mode: SSH default"
    make -f Makefile.mk PLATFORM=r36s CONFIG=debug __deploy_debug_on_r36s_with_ssh_default
    ;;
  ssh-explicit-key)
    echo "[R36S] PreLaunch mode: SSH explicit key"
    make -f Makefile.mk __deploy_debug_on_r36s_with_ssh_explicit_key
    ;;
  *)
    echo "[R36S] Unknown R36S_DEPLOY_MODE='$MODE' (expected: ssh-default | ssh-explicit-key)" >&2
    exit 2
    ;;
esac