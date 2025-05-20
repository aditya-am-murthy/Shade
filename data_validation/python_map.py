import os
import folium
import pandas as pd
from folium.plugins import HeatMap, MarkerCluster

# Folder where your 200 CSV files are stored
folder_path = '/Users/adityacode/Shade/july_csv'  # Update this with your folder path

# Define the fixed geographical bounds
LAT_MIN = 33.4
LAT_MAX = 34.3
LON_MIN = -118.6
LON_MAX = -117.6

# Function to load all CSV files from a folder
def load_data_from_folder(folder_path):
    # List to hold dataframes
    all_data = []
    
    # Iterate over each file in the folder
    for filename in os.listdir(folder_path):
        # Only process CSV files
        if filename.endswith('.csv'):
            file_path = os.path.join(folder_path, filename)
            df = pd.read_csv(file_path)
            if 'latitude' in df.columns and 'longitude' in df.columns:
                # Append only the latitude and longitude columns
                df = df[(df['speed'] < 3) & (df['local_location_at'].apply(lambda x: int(x.split(' ')[1].split(':')[0])) >= 8) & (df['local_location_at'].apply(lambda x: int(x.split(' ')[1].split(':')[0])) <= 11)]
                all_data.append(df[['latitude', 'longitude']])

    full_data = pd.concat(all_data,ignore_index=True)
    return full_data

# Create the map with OpenStreetMap as the base layer, focusing on the fixed region
def create_map():
    # Set the map center and zoom level based on the region bounds
    map_center = [(LAT_MIN + LAT_MAX) / 2, (LON_MIN + LON_MAX) / 2]  # Center of the region
    folium_map = folium.Map(location=map_center, zoom_start=12, tiles='OpenStreetMap')

    # Optionally, you can add a boundary rectangle around the region
    folium.Rectangle(
        bounds=[[LAT_MIN, LON_MIN], [LAT_MAX, LON_MAX]], 
        color="blue", 
        fill=False,
        weight=2
    ).add_to(folium_map)

    return folium_map

# Add heatmap (optimized for large datasets)
def add_heatmap(folium_map, df):
    # Use only a sample of the data for better performance (e.g., 10%)
    heat_data = df.sample(frac=0.1)[['latitude', 'longitude']].values.tolist()
    
    # Create and add the heatmap to the map
    HeatMap(heat_data).add_to(folium_map)

# Add marker clustering for large datasets
def add_marker_clustering(folium_map, df):
    # Create a marker cluster to handle the large number of points efficiently
    marker_cluster = MarkerCluster().add_to(folium_map)

    for _, row in df.iterrows():
        folium.Marker([row['latitude'], row['longitude']]).add_to(marker_cluster)

# Main function to combine everything
def plot_lat_lon_on_map():
    # Load the data from all the CSV files in the folder
    df = load_data_from_folder(folder_path)

    # Filter data to only include points within the defined bounds
    df = df[(df['latitude'] >= LAT_MIN) & (df['latitude'] <= LAT_MAX) &
            (df['longitude'] >= LON_MIN) & (df['longitude'] <= LON_MAX)]

    # Create the folium map
    folium_map = create_map()

    # Option 1: Add heatmap (this is better for density visualizations)
    add_heatmap(folium_map, df)

    # Option 2: Add marker clustering (this is better for individual point visibility)
    add_marker_clustering(folium_map, df)

    # Save the map to an HTML file
    folium_map.save("region_map_with_heat_and_clustering.html")
    print("Map saved as region_map_with_heat_and_clustering.html")

# Run the script
plot_lat_lon_on_map()
