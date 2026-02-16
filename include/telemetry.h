//=================================================================
// R36S Telemetry
//=================================================================
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>

#include "constants.h"

//=================================================================
// R36S uses Rockchip see specification link below
// https://rockchip.fr/RK3326%20datasheet%20V1.1.pdf
//=================================================================

#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// For R36S and Linux only STARTS:
//=================================================================
#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)

	//=================================================================
	// Telemetry Data
	//=================================================================
	typedef struct Telemetry
	{
		// CPU
		int r36s_cpu_core_count;
		float r36s_cpu_percent[MAX_CORES]; // 0..100
		float r36s_cpu_mhz[MAX_CORES];	   // current MHz
		float r36s_cpu_max_mhz[MAX_CORES]; // max MHz
		// scaling_governor (interactive =fast ramp-up for game)
		char r36s_governor[32];

		// Negative means not available
		float r36s_cpu_temp;
		float r36s_gpu_temp;

		// Memory
		float r36s_memory_total;
		float r36s_memory_available;
		float r36s_game_memory_usage;

		// Battery
		int r36s_battery_percent; // -1 if unavailable
		bool r36s_battery_charging;
		bool r36s_usb_power_connected;

		// Frame timing statistics (ms)
		float current_frame_ms;			  // current frame time
		float worst_frame_in_last_second; // worst frame in the last second
		int fps;						  // Game FPS

		// Sampling Timers
		double r36s_last_cpu_sample;
		double r36s_last_memory_sample;
		double r36s_last_temp_sample;
		double r36s_last_battery_sample;
		unsigned long long r36s_previous_idle[MAX_CORES];
		unsigned long long r36s_previous_total[MAX_CORES];
		bool cpu_sample_valid;
	} Telemetry;
	//=================================================================

	//=================================================================
	// Init Telemetry
	//=================================================================
	void InitTelemetry(Telemetry *r36s_telemetry);

	//=================================================================
	// Update
	//=================================================================
	void UpdateTelemetry(Telemetry *r36s_telemetry, double now);

	//=================================================================
	// Update Frame
	//=================================================================
	void UpdateTelemetryFrame(Telemetry *r36s_telemetry, float frame_time_sec, double now, int fps);

	//=================================================================
	// Draw overlay
	//=================================================================
	void DrawTelemetry(const Telemetry *r36s_telemetry,
					   int x, int y,
					   const char *glRenderer,
					   const char *glVersion,
					   const char *glslVersion);

//=================================================================
// R36S and Linux only ENDS:
//=================================================================
#endif

#ifdef __cplusplus
}
#endif

#endif // TELEMETRY_H