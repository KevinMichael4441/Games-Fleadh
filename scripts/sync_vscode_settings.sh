#!/usr/bin/env bash
#=================================================================
# Sync VSCode .vscode/settings.json from config/config.mk
#=================================================================
set -euo pipefail

# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/r36s_env.sh"

r36s_load_env

line_blue
info "Syncing VSCode Settings and rebuilding .vscode/settings.json"
line_blue

# Resolve paths
REPO_ROOT="${REPO_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
SETTINGS_FILE="$REPO_ROOT/.vscode/settings.json"

# Get values using Makefile
cd "$REPO_ROOT"

info "Reading configuration from Makefile.mk and /config/config.mk"

R36S_HOST_VALUE=$(make -f Makefile.mk print-r36s-host --no-print-directory 2>/dev/null || echo "")
R36S_GDB_PORT_VALUE=$(make -f Makefile.mk print-r36s-gdb-port --no-print-directory 2>/dev/null || echo "")

# Validate values
if [[ -z "$R36S_HOST_VALUE" ]]; then
	line_red
	error "Failed to read R36S_HOST from Makefile"
	warn "Ensure ./config/config.mk exists and contains R36S_HOST=<value>"
	line_red
	exit 1
fi

if [[ -z "$R36S_GDB_PORT_VALUE" ]]; then
	line_red
	error "Failed to read R36S_GDB_PORT from Makefile"
	warn "Ensure ./config/config.mk exists and contains R36S_GDB_PORT=<value>"
	line_red
	exit 1
fi

info_box "Configuration Values"
info "R36S Host: $R36S_HOST_VALUE"
info "GDB Port:  $R36S_GDB_PORT_VALUE"

# Create .vscode directory if needed (its in repo by default but just in case)
mkdir -p "$REPO_ROOT/.vscode"

# Update or create .vscode/settings.json
if [[ -f "$SETTINGS_FILE" ]]; then
	info "Updating existing .vscode/settings.json"
	
	# Check if jq is available
	if command -v jq &> /dev/null; then
		# Use jq (cleaner, preserves formatting)
		TMP=$(mktemp)
		if jq --arg host "$R36S_HOST_VALUE" --arg port "$R36S_GDB_PORT_VALUE" \
			'."r36s.host" = $host | ."r36s.gdb_port" = $port' \
			"$SETTINGS_FILE" > "$TMP" 2>/dev/null; then
			mv "$TMP" "$SETTINGS_FILE"
			success "Updated: $SETTINGS_FILE (using jq)"
		else
			rm -f "$TMP"
			warn "jq failed, falling back to sed"
			
			# Fallback to sed
			sed -i.bak \
				-e "s/\"r36s.host\": \".*\"/\"r36s.host\": \"$R36S_HOST_VALUE\"/" \
				-e "s/\"r36s.gdb_port\": \".*\"/\"r36s.gdb_port\": \"$R36S_GDB_PORT_VALUE\"/" \
				"$SETTINGS_FILE"
			rm -f "$SETTINGS_FILE.bak"
			success "Updated: $SETTINGS_FILE (using sed)"
		fi
	else
		# Fallback to sed
		sed -i.bak \
			-e "s/\"r36s.host\": \".*\"/\"r36s.host\": \"$R36S_HOST_VALUE\"/" \
			-e "s/\"r36s.gdb_port\": \".*\"/\"r36s.gdb_port\": \"$R36S_GDB_PORT_VALUE\"/" \
			"$SETTINGS_FILE"
		rm -f "$SETTINGS_FILE.bak"
		success "Updated: $SETTINGS_FILE"
	fi
else
	info "Creating .vscode/settings.json"
	
	# Create .vscode/settings.json
	cat > "$SETTINGS_FILE" << EOF
{
  "r36s.host": "$R36S_HOST_VALUE",
  "r36s.gdb_port": "$R36S_GDB_PORT_VALUE"
}
EOF
	success "Created: $SETTINGS_FILE"
fi

line_blue
warn "Reload VSCode window to apply changes"
info "Press: Ctrl+Shift+P -> 'Developer: Reload Window'"
line_blue

exit 0