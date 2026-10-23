#include "../include/hours.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_power.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PRINT_EVERY_ 30000 // Delay in milliseconds to print records in the file

const bool debug = true;

// const char *get_log_path() {
// 	static char path[PATH_MAX];
// 	const char *homedir = getenv("HOME");

// 	if (homedir == NULL) {
// 		homedir = getpwuid(getuid())->pw_dir;
// 	}

// 	snprintf(path, sizeof(path), "%s", homedir);
// 	return path;
// }

int get_log_directory(char *path, size_t size)
{
    const char *homedir = getenv("HOME");

    if (homedir == NULL || homedir[0] == '\0')
	{
        struct passwd *pw = getpwuid(getuid());

        if (pw == NULL || pw->pw_dir == NULL)
		{
            return -1;
        }

        homedir = pw->pw_dir;
    }

    int length = snprintf(path, size, "%s/battery-monitor-logs", homedir);

    if (length < 0 || (size_t)length >= size)
	{
        fprintf(stderr, "Log directory path is too long.\n");
        return -1;
    }

    if (mkdir(path, 0755) == -1 && errno != EEXIST)
	{
        perror("Could not create log directory");
        return -1;
    }

    struct stat info;

    if (stat(path, &info) == -1 || !S_ISDIR(info.st_mode))
	{
        fprintf(stderr, "Log path exists but is not a directory.\n");
        return -1;
    }

    return 0;
}

int main() {
    time_t rawtime;
    struct tm *timeinfo;

    // Get the current time in seconds since the Unix epoch
    time(&rawtime);

    // Convert to local time format
    timeinfo = localtime(&rawtime);

    char filename[PATH_MAX] = "";
	char log_directory[PATH_MAX];
	
	if (get_log_directory(log_directory, sizeof(log_directory)) != 0) {
        return 1;
    }
	int length = snprintf(
        filename,
        sizeof(filename),
        "%s/%d-%d-%d.log",
        log_directory,
        timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900
    );

	if (length < 0 || (size_t)length >= sizeof(filename)) {
        fprintf(stderr, "Log filename is too long.\n");
        return 1;
    }

    // snprintf(filename, sizeof(filename), "%s/", get_log_path());

    // char date_string[30];
    // snprintf(date_string, sizeof(date_string), "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

    if (debug) printf("%s\n", filename);

    // strcat(filename, date_string);
    // strcat(filename, ".log");

    if (debug) printf("File: %s\n", filename);

    SDL_Event event;
    bool running = true;

    Uint32 last_time = SDL_GetTicks();
    const Uint32 delay_interval = PRINT_EVERY_;

    int secs, pct;
    int old_power_state = getPowerState();



    FILE *recorded_data = fopen(filename, "a");
    if (recorded_data == NULL)
	{
        if (debug) printf("Can't open file in append mode");
        return -1;
    }
	// print once at start
	fprintf(recorded_data, "%d, %d, %d, %d, %d, %d, %d, %d, %d\n", getBatteryPercentage(), getPowerState(), secs, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_mday, timeinfo->tm_mon + 1, 1900 + timeinfo->tm_year);

	fflush(recorded_data);
	fclose(recorded_data);

	if (debug) printf("%d, %d, %d, %d, %d, %d, %d, %d, %d\n", getBatteryPercentage(), getPowerState(), secs, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_mday, timeinfo->tm_mon + 1, 1900 + timeinfo->tm_year);

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
		}

		Uint32 current_time = SDL_GetTicks();

		int current_power_state = getPowerState();
		if ((current_time - last_time > delay_interval) ||(old_power_state !=current_power_state)) // Only do this when the elapsed time is greater than delay_interval or the power state changes
		{
			// Update the time
			time(&rawtime);
			timeinfo = localtime(&rawtime);

			// printf("PWR_STT: %d, %d\n", current_power_state,
			// old_power_state);//Just for debugging

			last_time = current_time;
			SDL_PowerState state =
				SDL_GetPowerInfo(&secs, &pct); // Might be benefic in terms of
												// performance to only do this here
			if ((current_power_state != 3) ||(current_power_state !=old_power_state)) // Only record when it's not full and the state changed
			{

				FILE *recorded_data = fopen(filename, "a");
				if (recorded_data == NULL) {
					if (debug) printf("Can't open file in append mode");
					return -1;
				}

				// write in the file
				fprintf(recorded_data, "%d, %d, %d, %d, %d, %d, %d, %d, %d\n",
						getBatteryPercentage(), getPowerState(), secs,
						timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
						timeinfo->tm_mday, timeinfo->tm_mon + 1,
						1900 + timeinfo->tm_year);
				// force the writing immediately by flushing the buffer
				fflush(recorded_data);
				fclose(recorded_data);

				// print on the command line for debugging
				if (debug) printf("%d, %d, %d, %d, %d, %d, %d, %d, %d\n", getBatteryPercentage(), getPowerState(), secs, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_mday, timeinfo->tm_mon + 1, 1900 + timeinfo->tm_year);
				// format: percentage, state, remaining, hh, mm, ss, dd, MM, yy
			}

			old_power_state = current_power_state;
		}
		SDL_Delay(500);
  	}

  	return 0;
}