#!/usr/bin/env bash
set -euo pipefail

#=================================================================
# R36S_USER and R36S_HOST
#=================================================================
R36S_USER="${R36S_USER:-${1:-ark}}"
R36S_HOST="${R36S_HOST:-${2:-}}"

#=================================================================
# Need to know IP address of R36S
#=================================================================
if [ -z "$R36S_HOST" ]; then
	echo "ERROR: No R36S_HOST provided"
	echo "Usage:"
	echo "  export R36S_HOST=<device-ip>"
	echo "  export [R36S_USER=<user>]"
	echo "  bash $0"
	echo "  or"
	echo "  bash $0 [user] <device-ip>"
	exit 1
fi

echo "INFO: Installing SSH key on R36S ($R36S_USER@$R36S_HOST)"

KEY_PRIV="/root/.ssh/id_r36s"
KEY_PUB="/root/.ssh/id_r36s.pub"

if [ ! -f "$KEY_PUB" ]; then
	echo "ERROR: Missing public key: $KEY_PUB"
	exit 1
fi

echo "INFO: Installing SSH key on R36S ($R36S_USER@$R36S_HOST)"

PUBKEY="$(cat "$KEY_PUB")"

#=================================================================
# Force ssh to use build key
#=================================================================
SSH="ssh -i $KEY_PRIV"

#=================================================================
# Push the key if it isn't already there
#=================================================================
$SSH "$R36S_USER@$R36S_HOST" "mkdir -p ~/.ssh && chmod 700 ~/.ssh && \
    touch ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys && \
    grep -qxF $(printf %q "$PUBKEY") ~/.ssh/authorized_keys || \
    echo $(printf %q "$PUBKEY") >> ~/.ssh/authorized_keys"

echo "SUCCESS: Key installed. You can now SSH to R36S without a password."
echo "IMPORTANT: Change the ArkOS username and Password. Only allow SSH with Key"
echo "Otherwise you have a SSH enabled device open on you network, this is a security risk"
