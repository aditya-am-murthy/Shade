#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gdal.h>
#include <ogr_srs_api.h>
#include <string.h>
#include <ctype.h>

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

// Function to process a polygon and assign population counts to grid cells
void process_polygon(const char* wkt, int population_count) {
    OGRGeometryH geom;
    char* wkt_copy = (char*)wkt;

    if(OGR_G_CreateFromWkt(&wkt_copy, NULL, &geom) != OGRERR_NONE) {
        fprintf(stderr, "Failed to parse WKT\n");
        return;
    }

    OGREnvelope env;
    OGR_G_GetEnvelope(geom, &env);

    int lat_min = (int)floor((env.MinY - LAT_MIN)/GRID_SIZE);
    int lat_max = (int)ceil((env.MaxY - LAT_MIN)/GRID_SIZE);
    int lon_min = (int)floor((env.MinX - LON_MIN)/GRID_SIZE);
    int lon_max = (int)ceil((env.MaxX - LON_MIN)/GRID_SIZE);

    lat_min = lat_min < 0 ? 0 : (lat_min >= GRID_ROWS ? GRID_ROWS-1 : lat_min);
    lat_max = lat_max < 0 ? 0 : (lat_max >= GRID_ROWS ? GRID_ROWS-1 : lat_max);
    lon_min = lon_min < 0 ? 0 : (lon_min >= GRID_COLS ? GRID_COLS-1 : lon_min);
    lon_max = lon_max < 0 ? 0 : (lon_max >= GRID_COLS ? GRID_COLS-1 : lon_max);

    OGRGeometryH grid_cell = OGR_G_CreateGeometry(wkbPolygon);
    OGRGeometryH ring = OGR_G_CreateGeometry(wkbLinearRing);

    for(int i = lat_min; i <= lat_max; i++) {
        for(int j = lon_min; j <= lon_max; j++) {
            double min_lon = LON_MIN + j * GRID_SIZE;
            double min_lat = LAT_MIN + i * GRID_SIZE;
            double max_lon = min_lon + GRID_SIZE;
            double max_lat = min_lat + GRID_SIZE;

            OGR_G_Empty(ring);
            OGR_G_AddPoint_2D(ring, min_lon, min_lat);
            OGR_G_AddPoint_2D(ring, max_lon, min_lat);
            OGR_G_AddPoint_2D(ring, max_lon, max_lat);
            OGR_G_AddPoint_2D(ring, min_lon, max_lat);
            OGR_G_AddPoint_2D(ring, min_lon, min_lat);

            OGR_G_Empty(grid_cell);
            OGR_G_AddGeometryDirectly(grid_cell, OGR_G_Clone(ring));

            OGRGeometryH intersection = OGR_G_Intersection(geom, grid_cell);
            if(intersection && !OGR_G_IsEmpty(intersection)) {
                double area = OGR_G_Area(intersection);
                double cell_area = GRID_SIZE * GRID_SIZE;
                grid_data[i][j] += (int)((area / cell_area) * population_count);
                OGR_G_DestroyGeometry(intersection);
            }
        }
    }

    OGR_G_DestroyGeometry(ring);
    OGR_G_DestroyGeometry(grid_cell);
    OGR_G_DestroyGeometry(geom);
}

// Modified trim function to handle quotes and whitespace
char* trim_field(char *str) {
    if (!str) return str;

    // Trim leading whitespace
    char *start = str;
    while (*start && (isspace((unsigned char)*start) || *start == '"')) start++;

    // Trim trailing whitespace and quotes
    char *end = start + strlen(start) - 1;
    while (end > start && (isspace((unsigned char)*end) || *end == '"')) end--;

    *(end + 1) = '\0';
    return start;
}

// Enhanced CSV parser
int parse_csv_line(char *line, char **tract, char **population, char **polygon) {
    char *fields[3] = {NULL};
    int field_idx = 0;
    int in_quotes = 0;
    char *current = line;

    while (*current && field_idx < 3) {
        if (*current == '"') {
            in_quotes = !in_quotes;
            current++;
            continue;
        }

        if (*current == ',' && !in_quotes) {
            *current = '\0';
            fields[field_idx++] = trim_field(line);
            line = current + 1;
        }
        current++;
    }

    // Handle last field
    if (field_idx < 3) {
        fields[field_idx] = trim_field(line);
    }

    if (!fields[0] || !fields[1] || !fields[2]) return 0;

    *tract = fields[0];
    *population = fields[1];
    *polygon = fields[2];
    return 1;
}

// Function to validate if a string is a valid population number
int is_valid_population(const char *str) {
    char *endptr;
    strtod(str, &endptr);

    // Check if the string is a valid number (no additional characters after the number)
    return *endptr == '\0' && endptr != str;
}

// Function to read a long line that exceeds MAX_LINE_LENGTH
void read_long_line(FILE *file, char *line, size_t max_len) {
    size_t len = 0;
    int ch;

    // Read up to max_len characters or until newline
    while ((ch = fgetc(file)) != EOF && ch != '\n' && len < max_len) {
        line[len++] = (char)ch;
    }
    line[len] = '\0';  // Null-terminate the string

    // If the line exceeds the maximum length, continue reading until the full entry is retrieved
    if (len >= max_len) {
        while ((ch = fgetc(file)) != EOF && ch != '\n') {
            // Ensure the buffer is large enough to append another character
            if (len < max_len - 1) {
                line[len++] = (char)ch;
            }
        }

        // Null-terminate the string properly
        line[len] = '\0';
    }

    // Discard any remaining characters in the current line (until a newline or EOF)
    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        // Just discard the character
    }
}

int main() {
    GDALAllRegister();
    
    FILE* file = fopen("tract_to_pop.csv", "r");
    if (!file) {
        fprintf(stderr, "Failed to open file\n");
        return 1;
    }

    char line[MAX_LINE_LENGTH + 2];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        size_t len = strlen(line);

        // Handle long lines manually
        if (len >= MAX_LINE_LENGTH) {
            read_long_line(file, line, MAX_LINE_LENGTH);
        }

        char *tract, *population, *polygon;
        if (!parse_csv_line(line, &tract, &population, &polygon)) {
            fprintf(stderr, "Line %d: Invalid format - raw line: '%.50s...'\n", 
                    line_number, line);
            continue;
        }

        // Skip lines where population is invalid (e.g., coordinates instead of a number)
        if (!is_valid_population(population)) {
            fprintf(stderr, "Line %d: Invalid population value '%s'\n", 
                    line_number, population);
            continue;
        }

        // Validate population (accepts integer values with .0 decimal)
        double pop_value = strtod(population, NULL);
        process_polygon(polygon, (int)pop_value);
    }

    fclose(file);

    // Output the grid data for verification
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            printf("%d ", grid_data[i][j]);
        }
        printf("\n");
    }

    // Export grid data to a file called cmap.txt
    FILE *f = fopen("cmap.txt", "w");
    if (f == NULL) {
        fprintf(stderr, "Error opening file!\n");
        return 1;
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
