#!/bin/bash

# Exit on any error
set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the root directory (Shade)
ROOT_DIR="$( cd "$SCRIPT_DIR/../../.." && pwd )"
JULY_CSV_DIR="$ROOT_DIR/july_csv"

echo "Testing Location Processor..."
echo "Script directory: $SCRIPT_DIR"
echo "Root directory: $ROOT_DIR"
echo "July CSV directory: $JULY_CSV_DIR"

# Check if july_csv directory exists
if [ ! -d "$JULY_CSV_DIR" ]; then
    echo -e "${RED}Error: july_csv directory not found at $JULY_CSV_DIR${NC}"
    exit 1
fi

# Check if july_csv directory has any CSV files
CSV_COUNT=$(find "$JULY_CSV_DIR" -name "*.csv" | wc -l)
if [ "$CSV_COUNT" -eq 0 ]; then
    echo -e "${RED}Error: No CSV files found in $JULY_CSV_DIR${NC}"
    exit 1
fi
echo -e "${GREEN}Found $CSV_COUNT CSV files in july_csv directory${NC}"

# List the first few CSV files
echo "Sample CSV files:"
find "$JULY_CSV_DIR" -name "*.csv" | head -n 3 | while read file; do
    echo "  $file ($(wc -l < "$file") lines)"
done

# Change to the script directory
cd "$SCRIPT_DIR"

# Compile the code
echo "Compiling..."
make clean
make

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed${NC}"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p paths

# Run the processor
echo "Running processor on july_csv directory..."
./location_processor "$JULY_CSV_DIR"

# Check if the processor ran successfully
if [ $? -ne 0 ]; then
    echo -e "${RED}Processor failed${NC}"
    exit 1
fi

# Verify output structure
echo "Verifying output structure..."

# Check if paths directory was created
if [ ! -d "paths" ]; then
    echo -e "${RED}Error: paths directory not created${NC}"
    exit 1
fi

# Count the number of grid directories
grid_dirs=$(find paths -type d | wc -l)
echo "Found $grid_dirs grid directories"

# Count the number of path files
path_files=$(find paths -type f -name "*.csv" | wc -l)
echo "Found $path_files path files"

# Check if we have any output
if [ $path_files -eq 0 ]; then
    echo -e "${RED}Error: No path files were generated${NC}"
    exit 1
fi

# Sample a few path files to verify content
echo "Sampling path files..."
for file in $(find paths -type f -name "*.csv" | head -n 3); do
    echo "Checking file: $file"
    # Check if file has header and at least one data point
    if [ $(wc -l < "$file") -lt 2 ]; then
        echo -e "${RED}Error: File $file is empty or missing data${NC}"
        exit 1
    fi
    # Display first few lines
    echo "First few lines of $file:"
    head -n 3 "$file"
done

echo -e "${GREEN}Test completed successfully!${NC}"
echo "Generated $path_files path files in $grid_dirs grid directories" 