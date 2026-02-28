#ifndef IP_ADDRESS
#define IP_ADDRESS

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(PLATFORM_WINDOWS)
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

	//=================================================================
	// Splits octets ###.###.###.###
	//=================================================================
	int ParseIPAddressAt(const char *s, int text_length, int *a, int *b, int *c, int *d);

	//=================================================================
	// 127.0.0.1 loopback
	// IsLoopback - Check if IP is loopback address
	//=================================================================
	int IsLoopback(int a);

	//=================================================================
	// Parse IP from string and check its not a loopback
	//=================================================================
	int ParseIPAddress(const char *text, char out[16], int non_loopback);

	//=================================================================
	// This function checks for IP Address
	// If you ran the SD Card Config Script then this creates a service autostart
	// This service starts up the game with a logfile see ./scripts/config_sd_card.sh
	// see Autostart Script
	// echo "R36S_HOST=$(hostname -I | awk '{print $1}')"
	// ReadIPFromShell handling errors
	//=================================================================
	int ReadIPFromShell(const char *cmd, char out[16], int non_loopback);

	//=================================================================
	// Fetch IP Address
	//=================================================================
	int GetIPAddressString(char out[16]);

#ifdef __cplusplus
}
#endif

#endif // IP_ADDRESS