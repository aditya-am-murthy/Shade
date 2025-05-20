#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <stdarg.h>
#include "../../../C_Custom_Files/hashmap.h"

#define MAX_SPEED 7.0  // Maximum speed in m/s (25 km/h)
#define MAX_TIME_DIFF 14400  // 4 hours in seconds
#define GRID_SIZE 0.01  // Grid size in degrees (approximately 1km)
#define MAX_LINE_LENGTH 1024
#define MAX_PATH_LINE_LENGTH 4096  // Increased buffer for path lines

// Debug logging function
static void debug_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// Comparison function for qsort
static int compare_timestamps(const void* a, const void* b) {
    const LocationPoint* pa = (const LocationPoint*)a;
    const LocationPoint* pb = (const LocationPoint*)b;
    return (pa->timestamp > pb->timestamp) - (pa->timestamp < pb->timestamp);
}

// Function to ensure a directory exists
static void ensure_directory_exists(const char* path) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0700);
            *p = '/';
        }
    }
    mkdir(tmp, 0700);
}

// Function to process a single CSV file
static void process_csv_file(const char* filename, HashMap* map) {
    debug_log("Processing file: %s", filename);
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        debug_log("Error opening file: %s", filename);
        return;
    }

    char line[MAX_LINE_LENGTH + 2];
    int line_count = 0;
    int valid_entries = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';  // Remove newline
        }

        // Parse CSV line using strtok
        char* advertiser_id = NULL;
        char* time_str = NULL;
        char* latitude_str = NULL;
        char* longitude_str = NULL;
        char* speed_str = NULL;
        
        int col = 0;
        char* token = strtok(line, ",");
        
        while (token != NULL && col < 11) {
            switch (col) {
                case 0:  // advertiser_id
                    advertiser_id = token;
                    break;
                case 3:  // timestamp
                    time_str = token;
                    break;
                case 4:  // latitude
                    latitude_str = token;
                    break;
                case 5:  // longitude
                    longitude_str = token;
                    break;
                case 10:  // speed
                    speed_str = token;
                    break;
            }
            token = strtok(NULL, ",");
            col++;
        }

        // Skip if we didn't get all required fields
        if (!advertiser_id || !time_str || !latitude_str || !longitude_str || !speed_str) {
            continue;
        }

        // Convert timestamp string to time_t
        struct tm tm = {0};
        if (strptime(time_str, "%Y-%m-%d %H:%M:%S", &tm) == NULL) {
            continue;
        }
        time_t timestamp = mktime(&tm);

        // Convert coordinates and speed
        double latitude = atof(latitude_str);
        double longitude = atof(longitude_str);
        double speed = atof(speed_str);

        // Skip if speed is too high
        if (speed >= MAX_SPEED) {
            continue;
        }

        // Store location in hashmap
        if (!hashmap_set(map, advertiser_id, timestamp, latitude, longitude, speed)) {
            continue;
        }
        valid_entries++;
    }

    debug_log("File %s: processed %d lines, stored %d entries", 
              filename, line_count, valid_entries);
    fclose(file);
}

// Function to process a single day directory
static void process_day_directory(const char* day_dir, HashMap* map) {
    debug_log("Processing day directory: %s", day_dir);
    
    DIR* dir = opendir(day_dir);
    if (!dir) {
        debug_log("Error opening directory: %s", day_dir);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", day_dir, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            continue;
        }

        if (S_ISREG(st.st_mode) && strstr(entry->d_name, ".csv") != NULL) {
            // Process CSV file
            process_csv_file(full_path, map);
        }
    }

    closedir(dir);
}

// Function to process all advertisers and create travel paths
static void process_advertiser_data(HashMap* map) {
    debug_log("Processing advertiser data from hashmap...");
    
    HashMapIterator* iterator = hashmap_iterator_create(map);
    if (!iterator) {
        debug_log("Error creating hashmap iterator");
        return;
    }

    char* advertiser_id;
    LocationArray* locations;
    int advertiser_count = 0;
    int total_paths = 0;

    // Create a map to store paths by grid location
    typedef struct {
        char* filename;
        FILE* file;
        int is_new;
    } GridFile;

    GridFile* grid_files = NULL;
    size_t grid_files_count = 0;
    size_t grid_files_capacity = 0;

    while (hashmap_iterator_next(iterator, &advertiser_id, &locations)) {
        advertiser_count++;
        debug_log("Processing advertiser %s with %zu locations", 
                 advertiser_id, locations->count);

        // Sort locations by timestamp
        qsort(locations->points, locations->count, sizeof(LocationPoint), compare_timestamps);

        // Process locations into travel paths
        for (size_t i = 0; i < locations->count; i++) {
            LocationPoint* start = &locations->points[i];
            size_t path_length = 1;

            // Find points within 4 hours
            for (size_t j = i + 1; j < locations->count; j++) {
                LocationPoint* current = &locations->points[j];
                if (current->timestamp - start->timestamp > MAX_TIME_DIFF) {
                    break;
                }
                path_length++;
            }

            if (path_length > 1) {
                // Create path directory structure
                char path[256];
                int lat_grid = (int)(start->latitude / GRID_SIZE);
                int lon_grid = (int)(start->longitude / GRID_SIZE);
                snprintf(path, sizeof(path), "paths/%d/%d", lat_grid, lon_grid);
                ensure_directory_exists(path);

                // Find or create grid file
                char filename[512];
                snprintf(filename, sizeof(filename), "%s/paths.csv", path);
                
                GridFile* grid_file = NULL;
                for (size_t k = 0; k < grid_files_count; k++) {
                    if (strcmp(grid_files[k].filename, filename) == 0) {
                        grid_file = &grid_files[k];
                        break;
                    }
                }

                if (!grid_file) {
                    // Check if file exists
                    struct stat st;
                    int file_exists = (stat(filename, &st) == 0);

                    // Add new grid file
                    if (grid_files_count >= grid_files_capacity) {
                        grid_files_capacity = grid_files_capacity == 0 ? 16 : grid_files_capacity * 2;
                        grid_files = realloc(grid_files, grid_files_capacity * sizeof(GridFile));
                    }

                    grid_file = &grid_files[grid_files_count++];
                    grid_file->filename = strdup(filename);
                    grid_file->file = fopen(filename, "a");
                    grid_file->is_new = !file_exists;

                    if (grid_file->is_new) {
                        fprintf(grid_file->file, "advertiser_id;start_timestamp;path_points\n");
                    }
                }

                if (grid_file->file) {
                    // Create path line
                    char path_line[MAX_PATH_LINE_LENGTH];
                    char* current = path_line;
                    int remaining = sizeof(path_line);

                    // Write advertiser_id and start timestamp
                    int written = snprintf(current, remaining, "%s;%ld;(", 
                                         advertiser_id, start->timestamp);
                    if (written >= 0 && written < remaining) {
                        current += written;
                        remaining -= written;
                    }

                    // Write all points in the path
                    for (size_t j = 0; j < path_length; j++) {
                        LocationPoint* point = &locations->points[i + j];
                        if (j > 0) {
                            written = snprintf(current, remaining, ",");
                            if (written >= 0 && written < remaining) {
                                current += written;
                                remaining -= written;
                            }
                        }
                        written = snprintf(current, remaining, "%.6f,%.6f,%.2f",
                                         point->latitude, point->longitude, point->speed);
                        if (written >= 0 && written < remaining) {
                            current += written;
                            remaining -= written;
                        }
                    }

                    // Close the path
                    written = snprintf(current, remaining, ")\n");
                    if (written >= 0 && written < remaining) {
                        current += written;
                        remaining -= written;
                    }

                    // Write the complete path line
                    fprintf(grid_file->file, "%s", path_line);
                    total_paths++;
                    debug_log("Added path for advertiser %s to %s", advertiser_id, filename);
                }
            }

            // Skip processed points
            i += path_length - 1;
        }
    }

    // Close all grid files
    for (size_t i = 0; i < grid_files_count; i++) {
        if (grid_files[i].file) {
            fclose(grid_files[i].file);
        }
        free(grid_files[i].filename);
    }
    free(grid_files);

    debug_log("Processed %d advertisers, created %d paths", advertiser_count, total_paths);
    hashmap_iterator_destroy(iterator);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    debug_log("Starting location processor...");
    debug_log("Input directory: %s", argv[1]);

    // Create base paths directory
    ensure_directory_exists("paths");
    debug_log("Created paths directory");

    // Get list of day directories
    DIR* root_dir = opendir(argv[1]);
    if (!root_dir) {
        debug_log("Error opening root directory: %s", argv[1]);
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(root_dir)) != NULL) {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char day_path[1024];
        snprintf(day_path, sizeof(day_path), "%s/%s", argv[1], entry->d_name);

        struct stat st;
        if (stat(day_path, &st) == -1 || !S_ISDIR(st.st_mode)) {
            continue;
        }

        debug_log("\nProcessing day: %s", entry->d_name);

        // Create new hashmap for this day
        HashMap* map = hashmap_create(1000);
        if (!map) {
            debug_log("Error creating hashmap for day %s", entry->d_name);
            continue;
        }

        // Process all CSV files in this day's directory
        process_day_directory(day_path, map);

        debug_log("Day %s: hashmap contains %zu entries", entry->d_name, map->size);

        // Process all advertisers and create travel paths
        process_advertiser_data(map);

        // Cleanup hashmap for this day
        hashmap_destroy(map);
        debug_log("Completed processing day: %s", entry->d_name);
    }

    closedir(root_dir);
    debug_log("Processing complete");
    return 0;
} 