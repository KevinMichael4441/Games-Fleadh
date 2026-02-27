#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env
r36s_require_host || exit $?

KEY_PUB="/root/.ssh/id_r36s.pub"

line_blue
info "Disable SSH public/private key on R36S"
info "Re-enable password authentication"
line_blue

info_box "Host: $R36S_USER@$R36S_HOST"
info "Using existing SSH control socket"
info "Control Socket: $R36S_CONTROL_SOCKET"
line_blue

# Hard requirement: Make pre-target must have established socket
r36s_require_connection || exit $?

if [[ ! -f "$KEY_PUB" ]]; then
  line_red
  error "Missing public key: $KEY_PUB"
  warn "Exact key removal requires the original public key"
  line_red
  exit 2
fi

PUBKEY="$(cat "$KEY_PUB")"

line_blue
info "Removing public key"
line_blue

ssh -o ControlPath="$R36S_CONTROL_SOCKET" \
    -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null \
    "$R36S_USER@$R36S_HOST" <<EOF
set -euo pipefail

AUTH=~/.ssh/authorized_keys
if [ -f "\$AUTH" ]; then
  tmp=\$(mktemp)
  grep -vxF $(printf %q "$PUBKEY") "\$AUTH" > "\$tmp" || true
  cat "\$tmp" > "\$AUTH"
  rm -f "\$tmp"
  chmod 600 "\$AUTH" || true
fi
EOF

success "Public key removed"

line_blue
info "Re-enabling password authentication (OpenSSH)"
line_blue

ssh -o ControlPath="$R36S_CONTROL_SOCKET" \
    -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null \
    "$R36S_USER@$R36S_HOST" <<'EOF'
set -euo pipefail

rm -f /etc/ssh/sshd_config.d/99-r36s-hardening.conf 2>/dev/null || true

if [ -f /etc/ssh/sshd_config ]; then
  sudo sh -c '
    CFG=/etc/ssh/sshd_config
    sed -i "s/^PasswordAuthentication.*/PasswordAuthentication yes/" "$CFG" || true
    sed -i "s/^KbdInteractiveAuthentication.*/KbdInteractiveAuthentication yes/" "$CFG" || true
    sed -i "s/^ChallengeResponseAuthentication.*/ChallengeResponseAuthentication yes/" "$CFG" || true
    grep -q "^PasswordAuthentication" "$CFG" || echo "PasswordAuthentication yes" >> "$CFG"
    grep -q "^KbdInteractiveAuthentication" "$CFG" || echo "KbdInteractiveAuthentication yes" >> "$CFG"
    grep -q "^ChallengeResponseAuthentication" "$CFG" || echo "ChallengeResponseAuthentication yes" >> "$CFG"
  '
fi

(systemctl restart sshd 2>/dev/null || systemctl restart ssh 2>/dev/null || \
 service ssh restart 2>/dev/null || service sshd restart 2>/dev/null || true)
EOF

success "Password authentication enabled"
line_blue

info "You may now SSH using password authentication"

exit 0