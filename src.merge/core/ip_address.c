#include "core/ip_address.h"

int ParseIPAddressAt(const char *s, int text_length, int *a, int *b, int *c, int *d)
{
	if (!s || text_length <= 0)
		return 0;

	int n = 0;
	int parts[4] = {0, 0, 0, 0};

	for (int i = 0; i < 4; i++)
	{
		int val = 0, digits = 0;

		// Parse digits with bounds checking
		while (isdigit((unsigned char)s[n]))
		{
			val = val * 10 + (s[n] - '0');
			n++;
			digits++;
			if (digits > 3)
				return 0; // Too many digits for an octet
		}

		// Validate octet
		if (digits == 0)
			return 0; // No digits found
		if (val < 0 || val > 255)
			return 0; // Out of valid IP range
		parts[i] = val;

		// Check for dot separator (except after last octet)
		if (i < 3)
		{
			if (s[n] != '.')
				return 0;
			n++;
		}
	}

	*a = parts[0];
	*b = parts[1];
	*c = parts[2];
	*d = parts[3];
	return n;
}

int IsLoopback(int a)
{
	return a == 127;
}

int ParseIPAddress(const char *text, char out[IP_ADDRESS_MAX_LEN], int non_loopback)
{
	if (!text || !out)
		return 0;

	int text_length = (int)strlen(text);

	for (int i = 0; text[i]; i++)
	{
		int a, b, c, d;
		int consumed = ParseIPAddressAt(&text[i], text_length, &a, &b, &c, &d);

		if (consumed > 0)
		{
			// Skip loopback if requested
			if (non_loopback && IsLoopback(a))
			{
				i += consumed - 1;
				continue;
			}
			snprintf(out, IP_ADDRESS_MAX_LEN, "%d.%d.%d.%d", a, b, c, d);
			return 1;
		}
	}
	return 0;
}

int ReadIPFromShell(const char *cmd, char out[16], int non_loopback)
{
	FILE *fp = POPEN(cmd, "r");
	if (!fp)
		return 0;

	char buf[1024];
	int found = 0;

	while (fgets(buf, (int)sizeof(buf), fp))
	{
		if (ParseIPAddress(buf, out, non_loopback))
		{
			found = 1;
			break;
		}
	}

	PCLOSE(fp);
	return found;
}

//=================================================================
// Tries to fetch IP Address of Device
// Handy for SSH or other networking later
// As emulationstation is not starting up as default
// this is a easy way to discover IP without reviewing log on
// Physical sd card
//=================================================================
int GetIPAddressString(char out[16])
{
	// Default loopback
	strncpy(out, "127.0.0.1", 16);
	out[15] = '\0';

#if defined(PLATFORM_WEB)
	strncpy(out, "Unknown", 16);
	out[15] = '\0';
	// loopback
	return 0;

#elif defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// hostname -I no parsing
	if (ReadIPFromShell("hostname -I 2>/dev/null", out, 1))
		return 1;

#elif defined(PLATFORM_WINDOWS)
	// Warning: very limited testing - tested using wine
	// Try ipconfig command
	if (ReadIPFromShell("ipconfig", out, 1))
		return 1;

	// Try powershell
	if (ReadIPFromShell(
			"powershell -Command \"(Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'Ethernet*' | Select-Object -First 1).IPAddress\"",
			out, 1))
		return 1;
	// loopback
	return 0;
#else
	// loopback
	return 0;
#endif
}
