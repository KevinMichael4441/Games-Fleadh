#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../scripts/r36s_env.sh"
r36s_load_env
r36s_require_host || exit $?

KEY_PRIV="/root/.ssh/id_r36s"
KEY_PUB="/root/.ssh/id_r36s.pub"

line_blue
info "Enable SSH public/private key on R36S"
info "Disable password authentication"
line_blue

info_box "Host: $R36S_USER@$R36S_HOST"
info "Using existing SSH control socket"
info "Control Socket: $R36S_CONTROL_SOCKET"
line_blue

# Hard requirement: Make pre-target must have established socket
r36s_require_connection || exit $?

# Validate keys exist
if [[ ! -f "$KEY_PUB" ]]; then
  line_red
  error "Missing public key: $KEY_PUB"
  warn "Keys must be generated during docker build"
  line_red
  exit 2
fi

PUBKEY="$(cat "$KEY_PUB")"

line_blue
info "Installing public key"
line_blue

ssh -o ControlPath="$R36S_CONTROL_SOCKET" \
    -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null \
    "$R36S_USER@$R36S_HOST" <<EOF
set -euo pipefail

mkdir -p ~/.ssh
chmod 700 ~/.ssh
touch ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys

grep -qxF $(printf %q "$PUBKEY") ~/.ssh/authorized_keys || \
  echo $(printf %q "$PUBKEY") >> ~/.ssh/authorized_keys
EOF

success "Public key installed"

line_blue
info "Disabling password authentication (OpenSSH)"
line_blue

ssh -o ControlPath="$R36S_CONTROL_SOCKET" \
    -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null \
    "$R36S_USER@$R36S_HOST" <<'EOF'
set -euo pipefail

if [ -d /etc/ssh/sshd_config.d ]; then
  sudo sh -c 'cat > /etc/ssh/sshd_config.d/99-r36s-hardening.conf <<CFG
PasswordAuthentication no
KbdInteractiveAuthentication no
ChallengeResponseAuthentication no
CFG'
else
  sudo sh -c '
    CFG=/etc/ssh/sshd_config
    touch "$CFG"
    sed -i "s/^PasswordAuthentication.*/PasswordAuthentication no/" "$CFG" || true
    sed -i "s/^KbdInteractiveAuthentication.*/KbdInteractiveAuthentication no/" "$CFG" || true
    sed -i "s/^ChallengeResponseAuthentication.*/ChallengeResponseAuthentication no/" "$CFG" || true
    grep -q "^PasswordAuthentication" "$CFG" || echo "PasswordAuthentication no" >> "$CFG"
    grep -q "^KbdInteractiveAuthentication" "$CFG" || echo "KbdInteractiveAuthentication no" >> "$CFG"
    grep -q "^ChallengeResponseAuthentication" "$CFG" || echo "ChallengeResponseAuthentication no" >> "$CFG"
  '
fi

(systemctl restart sshd 2>/dev/null || systemctl restart ssh 2>/dev/null || \
 service ssh restart 2>/dev/null || service sshd restart 2>/dev/null || true)
EOF

success "Password authentication disabled"
line_blue

info "SSH key auth is now required"
info "If docker is rebuilt, device must be reflashed per workflow"

exit 0