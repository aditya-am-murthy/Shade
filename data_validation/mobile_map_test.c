#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For directory operations

#define GRID_SIZE 0.02
#define LAT_MIN 33.4
#define LAT_MAX 34.3  // Fixed to include polygon
#define LON_MIN -118.6
#define LON_MAX -117.6

#define GRID_ROWS 45  // (34.3-33.4)/0.02 = 45
#define GRID_COLS 50
#define MAX_LINE_LENGTH 10000
#define MAX_FIELD_LENGTH 4096

int grid_data[GRID_ROWS][GRID_COLS] = {{0}};

// Function to map latitude and longitude to grid cell
void map_to_grid(double latitude, double longitude, int *row, int *col) {
    *row = (int)((latitude - LAT_MIN) / GRID_SIZE);
    *col = (int)((longitude - LON_MIN) / GRID_SIZE);
}

void process_csv_file(char *filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file\n");
        return;
    }

    char line[MAX_LINE_LENGTH + 2];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        int col = 0;
        line_number++;
        size_t len = strlen(line);

        // Extract the latitude and longitude from the line
        char *latitude_str = NULL;
        char *longitude_str = NULL;
        char *token = strtok(line, ",");
        char *time_str = NULL;
        char *speed_str = NULL;

        while (token != NULL && col < 11) {
            if (col == 3) {
                time_str = token;
            } else
            if (col == 4) {
                latitude_str = token;
            } else if (col == 5) {
                longitude_str = token;
            }
            else if (col == 10) {
                speed_str = token;
            }
            token = strtok(NULL, ",");
            col++;
        }
        if (time_str) {
            // Extract the hour portion from the time_str (which is in the format "HH:MM:SS")
            char *space = strchr(time_str, ' ');
            if (space != NULL) {
                // Move past the space to the beginning of the time portion
                char *time_portion = space + 1;

                // Extract the two digits before the first colon ':' (the hour part)
                char hour[3];  // To store the two digits + null terminator
                strncpy(hour, time_portion, 2);
                hour[2] = '\0';  // Null terminate the string
                
                // Convert the time to an integer (hour)
                int hour_int = atoi(hour);

                //conver the speed to a float
                float speed = atof(speed_str);

                // Check if the hour is between 20:00 (8 PM) or before 04:00 (4 AM)
                if ((hour_int >= 20 || hour_int < 4) && speed < 3 && speed > -3) {
                    
                    if (latitude_str && longitude_str) {
                        double latitude = atof(latitude_str);
                        double longitude = atof(longitude_str);
                        
                        // Check if the latitude and longitude are within the bounds
                        if (latitude >= LAT_MIN && latitude <= LAT_MAX &&
                            longitude >= LON_MIN && longitude <= LON_MAX) {
                            
                            int row, col;
                            map_to_grid(latitude, longitude, &row, &col);

                            // Increment the population count of the grid cell
                            if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
                                grid_data[row][col]++;
                            }
                        }
                    }
                } else {
                    continue;  // Skip the data if the time is not in the valid range
                }

                // Print the hour for debugging purposes
            } else {
                printf("Invalid timestamp format.\n");
            }
        }
    }

    fclose(file);
}

// Function to process all CSV files in a directory
void process_csv_files_in_directory(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(directory_path);
    if (!dir) {
        perror("Failed to open directory");
        return;
    }

    // Iterate over each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if the entry is a regular file (not a directory)
        if (entry->d_type == DT_REG) {
            // Construct the full file path
            char filepath[MAX_FIELD_LENGTH];
            snprintf(filepath, sizeof(filepath), "%s/%s", directory_path, entry->d_name);

            // Process the CSV file
            printf("Processing file: %s\n", filepath);
            process_csv_file(filepath);
        }
    }

    // Close the directory
    closedir(dir);
}

int main() {
    // Directory containing the CSV files
    const char *directory_path = "/Users/adityacode/Shade/july_csv";

    // Process all CSV files in the directory
    process_csv_files_in_directory(directory_path);

    // Print the grid data (for debugging purposes)
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            printf("%d ", grid_data[i][j]);
        }
        printf("\n");
    }

    //export grid data to a file called mmap.txt
    FILE *f = fopen("mmap.txt", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            fprintf(f, "%d ", grid_data[i][j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;
}