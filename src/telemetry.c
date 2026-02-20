#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)

#include <raylib.h>
#include <raymath.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "telemetry.h"
#include "ip_address.h"

//=================================================================
// IP Address
//=================================================================
char ip_address_str[IP_ADDRESS_MAX_LEN];
int has_ip_address 		= 0;
double last_ip_lookup 	= -1000.0;
int ip_lookups_left 	= 0;

//=================================================================
// Read unsigned Long Long
//=================================================================
static bool read_ull(const char *path, unsigned long long *out)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return false;
	unsigned long long v = 0;
	int ok = fscanf(f, "%llu", &v);
	fclose(f);
	if (ok != 1)
		return false;
	*out = v;
	return true;
}

//=================================================================
// Read integer
//=================================================================
static bool read_int(const char *path, int *out)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return false;
	int v = 0;
	int ok = fscanf(f, "%d", &v);
	fclose(f);
	if (ok != 1)
		return false;
	*out = v;
	return true;
}

//=================================================================
// Read line
//=================================================================
static bool read_line(const char *path, char *buffer, size_t buffer_length)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return false;
	if (!fgets(buffer, (int)buffer_length, f))
	{
		fclose(f);
		return false;
	}
	fclose(f);
	size_t core_count = strlen(buffer);
	while (core_count && (buffer[core_count - 1] == '\n' || buffer[core_count - 1] == '\r'))
		buffer[--core_count] = 0;
	return true;
}

//=================================================================
// Get CPU Core Count
//=================================================================
static int GetCores(void)
{
	long core_count = sysconf(_SC_NPROCESSORS_ONLN);
	if (core_count < 1)
		core_count = 1;
	if (core_count > MAX_CORES)
		core_count = MAX_CORES;
	return (int)core_count;
}

//=================================================================
// Short format OpenGL Version
//=================================================================
static void ShortGLVersion(const char *in, char *out, int outSize)
{
	if (!in)
	{
		snprintf(out, outSize, "(null)");
		return;
	}

	const char *p = strstr(in, "OpenGL ES ");
	if (!p)
	{
		snprintf(out, outSize, "%.*s", outSize - 1, in);
		return;
	}

	char w1[32] = {0}, w2[32] = {0}, w3[32] = {0};
	if (sscanf(p, "%31s %31s %31s", w1, w2, w3) == 3)
		snprintf(out, outSize, "%s %s %s", w1, w2, w3);
	else
		snprintf(out, outSize, "%.*s", outSize - 1, in);
}

//=================================================================
// CPU Usage
//=================================================================
static void CPUUsagePercentPerCoreTelemetry(Telemetry *r36s_telemetry)
{
	FILE *f = fopen("/proc/stat", "r");
	if (!f)
		return;

	char line[256];
	int cores_parsed = 0;

	while (fgets(line, sizeof(line), f))
	{
		if (strncmp(line, "cpu", 3) != 0)
			break;

		if (line[3] < '0' || line[3] > '9')
			continue; // skip aggregate CPU

		int core_index = -1;
		unsigned long long user_ticks = 0, nice_ticks = 0, system_ticks = 0;
		unsigned long long idle_ticks = 0, iowait_ticks = 0;
		unsigned long long irq_ticks = 0, softirq_ticks = 0, steal_ticks = 0;

		int scanned = sscanf(line,
							 "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu",
							 &core_index,
							 &user_ticks, &nice_ticks, &system_ticks,
							 &idle_ticks, &iowait_ticks,
							 &irq_ticks, &softirq_ticks, &steal_ticks);

		if (scanned < 5)
			continue;

		if (core_index < 0 || core_index >= r36s_telemetry->r36s_cpu_core_count)
			continue;

		unsigned long long idle_total_ticks = idle_ticks + iowait_ticks;
		unsigned long long active_total_ticks =
			user_ticks + nice_ticks + system_ticks +
			irq_ticks + softirq_ticks + steal_ticks;

		unsigned long long total_ticks =
			idle_total_ticks + active_total_ticks;

		if (r36s_telemetry->cpu_sample_valid)
		{
			unsigned long long prev_idle_ticks =
				r36s_telemetry->r36s_previous_idle[core_index];
			unsigned long long prev_total_ticks =
				r36s_telemetry->r36s_previous_total[core_index];

			unsigned long long delta_total_ticks =
				(total_ticks >= prev_total_ticks)
					? (total_ticks - prev_total_ticks)
					: 0;

			unsigned long long delta_idle_ticks =
				(idle_total_ticks >= prev_idle_ticks)
					? (idle_total_ticks - prev_idle_ticks)
					: 0;

			float cpu_usage_pct = 0.0f;
			if (delta_total_ticks > 0)
				cpu_usage_pct =
					(float)(delta_total_ticks - delta_idle_ticks) * 100.0f /
					(float)delta_total_ticks;

			r36s_telemetry->r36s_cpu_percent[core_index] =
				Clamp(cpu_usage_pct, 0.0f, 100.0f);
		}

		r36s_telemetry->r36s_previous_idle[core_index] = idle_total_ticks;
		r36s_telemetry->r36s_previous_total[core_index] = total_ticks;

		cores_parsed++;
		if (cores_parsed >= r36s_telemetry->r36s_cpu_core_count)
			break;
	}

	fclose(f);
	r36s_telemetry->cpu_sample_valid = true;
}

//=================================================================
// CPU Governor
//=================================================================
static void CPUGovernorTelemetry(Telemetry *r36s_telemetry)
{
	// max freq per core
	for (int i = 0; i < r36s_telemetry->r36s_cpu_core_count; i++)
	{
		char path[160];
		snprintf(path, sizeof(path),
				 "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);

		unsigned long long freq_kHz = 0;
		if (read_ull(path, &freq_kHz) && freq_kHz > 0)
			r36s_telemetry->r36s_cpu_max_mhz[i] = (float)freq_kHz / 1000.0f;
		else
			r36s_telemetry->r36s_cpu_max_mhz[i] = 0.0f;
	}

	// governor from cpu0
	r36s_telemetry->r36s_governor[0] = 0;
	if (!read_line("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor",
				   r36s_telemetry->r36s_governor, sizeof(r36s_telemetry->r36s_governor)))
	{
		snprintf(r36s_telemetry->r36s_governor, sizeof(r36s_telemetry->r36s_governor), "unknown");
	}
}

//=================================================================
// CPU Clocks
//=================================================================
static void CPUCoreClocksTelemetry(Telemetry *r36s_telemetry)
{
	for (int i = 0; i < r36s_telemetry->r36s_cpu_core_count; i++)
	{
		char path[160];
		snprintf(path, sizeof(path),
				 "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", i);

		unsigned long long freq_kHz = 0;
		if (read_ull(path, &freq_kHz) && freq_kHz > 0)
			r36s_telemetry->r36s_cpu_mhz[i] = (float)freq_kHz / 1000.0f;
		else
			r36s_telemetry->r36s_cpu_mhz[i] = 0.0f;
	}
}

//=================================================================
// Temp degrees C
//=================================================================
static float ReadTemperatureCelsius(const char *path)
{
	int millidegrees_celsius = 0;
	if (!read_int(path, &millidegrees_celsius))
		return -1.0f;
	return (float)millidegrees_celsius / 1000.0f;
}

//=================================================================
// CPU and GPU Temp
//=================================================================
static void TempsTelemetry(Telemetry *r36s_telemetry)
{
	r36s_telemetry->r36s_cpu_temp = ReadTemperatureCelsius("/sys/class/thermal/thermal_zone0/temp");
	r36s_telemetry->r36s_gpu_temp = ReadTemperatureCelsius("/sys/class/thermal/thermal_zone1/temp");
}

//=================================================================
// Memory Data
//=================================================================
static void MemoryTelemetry(Telemetry *r36s_telemetry)
{
	FILE *f = fopen("/proc/meminfo", "r");
	if (!f)
		return;

	unsigned long long memory_total_KiB = 0, memory_avail_KiB = 0;
	char key[64];
	unsigned long long value = 0;

	while (fscanf(f, "%63s %llu %*s", key, &value) == 2)
	{
		if (strcmp(key, "MemTotal:") == 0)
			memory_total_KiB = value;
		else if (strcmp(key, "MemAvailable:") == 0)
			memory_avail_KiB = value;

		if (memory_total_KiB && memory_avail_KiB)
			break;
	}
	fclose(f);

	r36s_telemetry->r36s_memory_total = memory_total_KiB ? (float)memory_total_KiB / 1024.0f : 0.0f;
	r36s_telemetry->r36s_memory_available = memory_avail_KiB ? (float)memory_avail_KiB / 1024.0f : 0.0f;

	// Game RSS (Resident Set Size): /proc/self/status -> VmRSS (KiB)
	r36s_telemetry->r36s_game_memory_usage = 0.0f;
	f = fopen("/proc/self/status", "r");
	if (!f)
		return;

	char line[256];
	while (fgets(line, sizeof(line), f))
	{
		if (strncmp(line, "VmRSS:", 6) == 0)
		{
			unsigned long long physical_ram_rss_KiB = 0;
			if (sscanf(line, "VmRSS: %llu kB", &physical_ram_rss_KiB) == 1)
				r36s_telemetry->r36s_game_memory_usage = (float)physical_ram_rss_KiB / 1024.0f;
			break;
		}
	}
	fclose(f);
}

//=================================================================
// Battery Data
//=================================================================
static void BatteryTelemetry(Telemetry *r36s_telemetry)
{
	// Value
	int value = 0;

	// Battery percentage
	if (read_int("/sys/class/power_supply/battery/capacity", &value))
		r36s_telemetry->r36s_battery_percent = (int)Clamp((float)value, 0.0f, 100.0f);
	else
		r36s_telemetry->r36s_battery_percent = -1;

	char battery_status[64];
	if (read_line("/sys/class/power_supply/battery/status", battery_status, sizeof(battery_status)))
		r36s_telemetry->r36s_battery_charging = (strcmp(battery_status, "Charging") == 0);
	else
		r36s_telemetry->r36s_battery_charging = false;

	// Charging
	if (read_int("/sys/class/power_supply/usb/online", &value))
		r36s_telemetry->r36s_usb_power_connected = (value != 0);
	else
		r36s_telemetry->r36s_usb_power_connected = false;
}

//=================================================================
// Gameloop e.g timings
//=================================================================
static void UpdateGameLoopTelemetry(Telemetry *r36s_telemetry, double now)
{
	static double game_loop_start_time = 0.0;
	static float game_loop_max_frame_ms = 0.0f;

	if (game_loop_start_time == 0.0)
		game_loop_start_time = now;

	if (r36s_telemetry->current_frame_ms > game_loop_max_frame_ms)
		game_loop_max_frame_ms = r36s_telemetry->current_frame_ms;

	if ((now - game_loop_start_time) >= 1.0)
	{
		r36s_telemetry->worst_frame_in_last_second = game_loop_max_frame_ms;
		game_loop_start_time = now;
		game_loop_max_frame_ms = 0.0f;
	}
}

//=================================================================
// Init Telemetry
//=================================================================
void InitTelemetry(Telemetry *r36s_telemetry)
{
	memset(r36s_telemetry, 0, sizeof(*r36s_telemetry));

	r36s_telemetry->r36s_cpu_core_count = GetCores();

	// mark unavailable until sampled
	r36s_telemetry->r36s_cpu_temp = -1.0f;
	r36s_telemetry->r36s_gpu_temp = -1.0f;
	r36s_telemetry->r36s_battery_percent = -1;

	// force first samples immediately
	r36s_telemetry->r36s_last_cpu_sample = CPU_SAMPLE_UNINITIALIZED;
	r36s_telemetry->r36s_last_memory_sample = CPU_SAMPLE_UNINITIALIZED;
	r36s_telemetry->r36s_last_temp_sample = CPU_SAMPLE_UNINITIALIZED;
	r36s_telemetry->r36s_last_battery_sample = CPU_SAMPLE_UNINITIALIZED;

	r36s_telemetry->cpu_sample_valid = false;

	CPUGovernorTelemetry(r36s_telemetry);

	// Get IP Address
	has_ip_address = GetIPAddressString(ip_address_str);
	if (!has_ip_address)
		ip_lookups_left = IP_ADDRESS_MAX_RETRIES;
	else
		ip_lookups_left = 0;
}

//=================================================================
// Update Frame
//=================================================================
void UpdateTelemetryFrame(Telemetry *r36s_telemetry, float frame_time_sec, double now, int fps)
{
	r36s_telemetry->current_frame_ms = frame_time_sec * 1000.0f;
	r36s_telemetry->fps = fps;

	// Fetch IP Address
	// Only try once per second until we have a non-loopback IP.
	// (If you want it to update forever, remove `!have_ip`.)
	if (!has_ip_address && ip_lookups_left > 0 && (now - last_ip_lookup) > IP_ADDRESS_MAX_RETRY_INTERVAL)
	{
		last_ip_lookup = now;
		ip_lookups_left--;
		has_ip_address = GetIPAddressString(ip_address_str);
	}
}

//=================================================================
// Update Telemetry
//=================================================================
void UpdateTelemetry(Telemetry *r36s_telemetry, double now)
{
	UpdateGameLoopTelemetry(r36s_telemetry, now);

	// CPU: 4 Hz
	if ((now - r36s_telemetry->r36s_last_cpu_sample) >= 0.25)
	{
		CPUUsagePercentPerCoreTelemetry(r36s_telemetry);
		CPUCoreClocksTelemetry(r36s_telemetry);
		r36s_telemetry->r36s_last_cpu_sample = now;
	}

	// Memory: 2 Hz
	if ((now - r36s_telemetry->r36s_last_memory_sample) >= 0.5)
	{
		MemoryTelemetry(r36s_telemetry);
		r36s_telemetry->r36s_last_memory_sample = now;
	}

	// Temps: 1 Hz
	if ((now - r36s_telemetry->r36s_last_temp_sample) >= 1.0)
	{
		TempsTelemetry(r36s_telemetry);
		r36s_telemetry->r36s_last_temp_sample = now;
	}

	// Battery: 0.5 Hz
	if ((now - r36s_telemetry->r36s_last_battery_sample) >= 2.0)
	{
		BatteryTelemetry(r36s_telemetry);
		r36s_telemetry->r36s_last_battery_sample = now;
	}
}

//=================================================================
// Color range scale: green -> orange -> red.
//=================================================================
static Color BarColorScale(float percent)
{
	percent = Clamp(percent, 0.0f, 100.0f);

	// Green <= 25%
	float scale = (percent <= 25.0f) ? 0.0f : (percent - 25.0f) / 75.0f; // 0..1

	// Green -> Red
	unsigned char r = (unsigned char)(255.0f * scale);
	unsigned char g = (unsigned char)(255.0f * (1.0f - scale));
	return (Color){r, g, 0, 255};
}

//=================================================================
// Battery 100% Good Color
//=================================================================
static Color BarColorScaleInverted(float percent)
{
	percent = Clamp(percent, 0.0f, 100.0f);
	float inverse = 100.0f - percent;
	return BarColorScale(inverse);
}

//=================================================================
// Draw battery level
//=================================================================
static void DrawBatteryBar(int x, int y, int w, int h, float percent)
{
	percent = Clamp(percent, 0.0f, 100.0f);
	Color color = BarColorScaleInverted(percent);

	DrawRectangleLines(x, y, w, h, RAYWHITE);

	int fill_width = (int)((percent / 100.0f) * (float)(w - 2));
	if (fill_width < 0)
		fill_width = 0;
	if (fill_width > (w - 2))
		fill_width = (w - 2);

	DrawRectangle(x + 1, y + 1, fill_width, h - 2, color);
}

//=================================================================
// Draw bar for CPU or Memory usage
//=================================================================
static void DrawTelemetryBar(int x, int y, int w, int h, float percent)
{
	percent = Clamp(percent, 0.0f, 100.0f);
	Color color = BarColorScale(percent);

	DrawRectangleLines(x, y, w, h, RAYWHITE);
	int fill_width = (int)((percent / 100.0f) * (float)(w - 2));
	if (fill_width < 0)
		fill_width = 0;
	if (fill_width > (w - 2))
		fill_width = (w - 2);

	DrawRectangle(x + 1, y + 1, fill_width, h - 2, color);
}

//=================================================================
// Draw Telemetry Panel
//=================================================================
void DrawTelemetry(const Telemetry *r36s_telemetry,
				   int x, int y,
				   const char *glRenderer,
				   const char *glVersion,
				   const char *glslVersion)
{
	const int line_height = SMALL_FONT_SIZE + 4;
	const int bar_width = 110;
	const int bar_height = 10;
	const int panel_width = 620;

	// Line count
	int lines = 0;
	lines += 1;									  // Title
	lines += 2;									  // GPU/GL header
	lines += 2;									  // Frames & Governor
	lines += r36s_telemetry->r36s_cpu_core_count; // Cores
	lines += 1;									  // Temps
	lines += 1;									  // Memory
	lines += 1;									  // Battery
	lines += 1;									  // IP Address
	lines += 1;									  // Sampling
	int panel_height = 10 + lines * line_height + 8;

	DrawRectangle(x, y, panel_width, panel_height, Fade(BLACK, 0.60f));
	DrawRectangleLines(x, y, panel_width, panel_height, RAYWHITE);

	int pos_x = x + 8;
	int pos_y = y + 6;

	DrawText("Game Telemetry", pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	char gl_short_description[64];
	ShortGLVersion(glVersion, gl_short_description, sizeof(gl_short_description));

	DrawText(TextFormat("GPU: %s", glRenderer ? glRenderer : "(null)"), pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	DrawText(TextFormat("GL: %s   |   GLSL: %s", gl_short_description, glslVersion ? glslVersion : "(null)"),
			 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	DrawText(TextFormat("Frame: %.1f ms (worst 1s: %.1f)   FPS: %d",
						r36s_telemetry->current_frame_ms, r36s_telemetry->worst_frame_in_last_second, r36s_telemetry->fps),
			 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	DrawText(TextFormat("CPU: %d cores  Gov: %s   Max: %.0f MHz",
						r36s_telemetry->r36s_cpu_core_count,
						r36s_telemetry->r36s_governor[0] ? r36s_telemetry->r36s_governor : "unknown",
						(r36s_telemetry->r36s_cpu_max_mhz[0] > 0.0f) ? r36s_telemetry->r36s_cpu_max_mhz[0] : 0.0f),
			 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	// CPU lines
	for (int i = 0; i < r36s_telemetry->r36s_cpu_core_count; i++)
	{
		float cpu_max_mhz = r36s_telemetry->r36s_cpu_max_mhz[i];
		float cpu_current_mhz = r36s_telemetry->r36s_cpu_mhz[i];
		float cpu_clock_utilize_percent = (cpu_max_mhz > 0.0f) ? (cpu_current_mhz / cpu_max_mhz) * 100.0f : 0.0f;

		// Heavy Load check clock is below max
		bool cpu_is_throttled = (r36s_telemetry->r36s_cpu_percent[i] > 80.0f) && (cpu_max_mhz > 0.0f) && (cpu_clock_utilize_percent < 85.0f);

		DrawText(TextFormat("CPU%d: %3.0f%%  %4.0f/%4.0f MHz%s",
							i, r36s_telemetry->r36s_cpu_percent[i], cpu_current_mhz, cpu_max_mhz, cpu_is_throttled ? " !" : ""),
				 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);

		DrawTelemetryBar(x + panel_width - (bar_width + 12), pos_y + 4, bar_width, bar_height, r36s_telemetry->r36s_cpu_percent[i]);
		pos_y += line_height;
	}

	// Temps
	if (r36s_telemetry->r36s_cpu_temp >= 0 && r36s_telemetry->r36s_gpu_temp >= 0)
		DrawText(TextFormat("Temp: CPU %.1f C   GPU %.1f C", r36s_telemetry->r36s_cpu_temp, r36s_telemetry->r36s_gpu_temp),
				 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	else if (r36s_telemetry->r36s_cpu_temp >= 0)
		DrawText(TextFormat("Temp: CPU %.1f C", r36s_telemetry->r36s_cpu_temp), pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	else if (r36s_telemetry->r36s_gpu_temp >= 0)
		DrawText(TextFormat("Temp: GPU %.1f C", r36s_telemetry->r36s_gpu_temp), pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	else
		DrawText("Temp: (unavailable)", pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	// Memory
	if (r36s_telemetry->r36s_memory_total > 0.0f)
	{
		// Memory not available for allocations
		float system_memory_used_mib = r36s_telemetry->r36s_memory_total - r36s_telemetry->r36s_memory_available;
		float memory_used_percent = (system_memory_used_mib / r36s_telemetry->r36s_memory_total) * 100.0f;

		DrawText(TextFormat("Memory: %.0f/%.0f MiB (%.0f%%)   Avail %.0f   Game %.0f",
							system_memory_used_mib, r36s_telemetry->r36s_memory_total, memory_used_percent,
							r36s_telemetry->r36s_memory_available, r36s_telemetry->r36s_game_memory_usage),
				 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);

		DrawTelemetryBar(x + panel_width - (bar_width + 12), pos_y + 4, bar_width, bar_height, memory_used_percent);
	}
	else
	{
		DrawText("Memory: (unavailable)", pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	}
	pos_y += line_height;

	// Battery
	if (r36s_telemetry->r36s_battery_percent >= 0)
	{
		const char *state = (r36s_telemetry->r36s_usb_power_connected || r36s_telemetry->r36s_battery_charging) ? "Charging" : "Discharging";
		DrawText(TextFormat("Battery: %d%%   %s", r36s_telemetry->r36s_battery_percent, state),
				 pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
		DrawBatteryBar(x + panel_width - (bar_width + 12), pos_y + 4, bar_width, bar_height, (float)r36s_telemetry->r36s_battery_percent);
	}
	else
	{
		DrawText("Battery: (unavailable)", pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	}
	pos_y += line_height;

	// Draw IP Address
	DrawText(TextFormat("IP Address: %s", ip_address_str), pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
	pos_y += line_height;

	DrawText("Sampling: CPU 4 Hz | Mem 2 Hz | Temp 1 Hz | Batt 0.5 Hz", pos_x, pos_y, SMALL_FONT_SIZE, RAYWHITE);
}
#endif // R36S and Linux only