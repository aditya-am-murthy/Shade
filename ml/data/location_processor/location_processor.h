#ifndef LOCATION_PROCESSOR_H
#define LOCATION_PROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../C_Custom_Files/hashmap.h"

// Constants for LA area boundaries
#define LAT_MIN 33.4
#define LAT_MAX 34.3
#define LON_MIN -118.6
#define LON_MAX -117.6
#define GRID_INCREMENT 0.01
#define MAX_TIME_DIFF 14400  // 4 hours in seconds
#define MAX_SPEED 7.0       // Maximum speed threshold

// Structure to hold a single location ping
typedef struct {
    char advertiser_id[64];
    time_t location_at;
    double latitude;
    double longitude;
    double altitude;
    double horizontal_accuracy;
    double vertical_accuracy;
    double heading;
    double speed;
} LocationPing;

// Structure to hold a travel path
typedef struct {
    LocationPing* pings;
    size_t count;
    size_t capacity;
} TravelPath;

// Structure for the hashmap entry
typedef struct {
    char advertiser_id[64];
    LocationPing* pings;
    size_t count;
    size_t capacity;
} AdvertiserData;

// Function declarations
void process_csv_file(const char* filename, HashMap* map);
void process_directory(const char* dir_path, HashMap* map);
void create_grid_structure(void);
void save_travel_path(const TravelPath* path);
void process_advertiser_data(HashMap* map, const char* advertiser_id);
void free_travel_path(TravelPath* path);

#endif // LOCATION_PROCESSOR_H 