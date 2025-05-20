import os
import folium
import pandas as pd
from folium.plugins import HeatMap, MarkerCluster
import numpy as np
from datetime import datetime, timedelta

# Define the fixed geographical bounds
LAT_MIN = 33.4
LAT_MAX = 34.3
LON_MIN = -118.6
LON_MAX = -117.6

# Function to load data for specific time range and day
def load_data_for_time_range(day, start_hour, end_hour, next_day_data=False):
    folder_path = f'/Users/adityacode/Shade/july_csv/{day}'
    all_data = []
    
    for filename in os.listdir(folder_path):
        if filename.endswith('.csv'):
            file_path = os.path.join(folder_path, filename)
            df = pd.read_csv(file_path)
            if 'latitude' in df.columns and 'longitude' in df.columns and 'local_location_at' in df.columns:
                # Parse timestamp and filter
                df['datetime'] = pd.to_datetime(df['local_location_at'])
                df['hour'] = df['datetime'].dt.hour
                df['day'] = df['datetime'].dt.day
                
                # Filter based on speed, time range, and bounds
                if not next_day_data:
                    df_filtered = df[(df['speed'] < 3) & 
                                  (df['hour'] >= start_hour) & 
                                  (df['hour'] < end_hour) &
                                  (df['day'] == day)]
                else:
                    # For the 20-24 range, include 0-2 from next day
                    df_filtered = df[(df['speed'] < 3) & 
                                  (((df['hour'] >= start_hour) & (df['hour'] < end_hour) & (df['day'] == day)) |
                                   ((df['hour'] >= 0) & (df['hour'] < 2) & (df['day'] == day + 1)))]
                
                all_data.append(df_filtered[['latitude', 'longitude']])

    return pd.concat(all_data, ignore_index=True) if all_data else pd.DataFrame(columns=['latitude', 'longitude'])

# Calculate density for heatmap scaling
def calculate_density(df, max_density):
    if df.empty:
        return []
    
    lat_bins = np.linspace(LAT_MIN, LAT_MAX, 100)
    lon_bins = np.linspace(LON_MIN, LON_MAX, 100)
    hist, lat_edges, lon_edges = np.histogram2d(df['latitude'], df['longitude'], bins=[lat_bins, lon_bins])
    
    weights = []
    for lat, lon in zip(df['latitude'], df['longitude']):
        lat_idx = np.digitize(lat, lat_bins) - 1
        lon_idx = np.digitize(lon, lon_bins) - 1
        lat_idx = min(max(lat_idx, 0), hist.shape[0] - 1)
        lon_idx = min(max(lon_idx, 0), hist.shape[1] - 1)
        weight = hist[lat_idx, lon_idx] / max_density
        weights.append(weight)
    
    heat_data = [[lat, lon, w] for lat, lon, w in zip(df['latitude'], df['longitude'], weights)]
    return heat_data

# Create the map with OpenStreetMap as the base layer
def create_map():
    map_center = [(LAT_MIN + LAT_MAX) / 2, (LON_MIN + LON_MAX) / 2]
    folium_map = folium.Map(location=map_center, zoom_start=12, tiles='OpenStreetMap')
    
    folium.Rectangle(
        bounds=[[LAT_MIN, LON_MIN], [LAT_MAX, LON_MAX]], 
        color="blue", 
        fill=False,
        weight=2
    ).add_to(folium_map)
    
    return folium_map

# Add heatmap with absolute scaling
def add_heatmap(folium_map, df, max_density):
    if not df.empty:
        heat_data = calculate_density(df, max_density)
        HeatMap(
            heat_data,
            min_opacity=0.2,
            radius=15,
            blur=20,
            gradient={
                "0.0": 'blue',  # Use strings as keys
                "0.3": 'cyan',
                "0.5": 'lime',
                "0.7": 'yellow',
                "0.9": 'orange',
                "1.0": 'red'
            }
        ).add_to(folium_map)

# Add marker clustering
def add_marker_clustering(folium_map, df):
    if not df.empty:
        marker_cluster = MarkerCluster().add_to(folium_map)
        sample_df = df.sample(frac=0.1)  # Sample to prevent overcrowding
        for _, row in sample_df.iterrows():
            folium.Marker([row['latitude'], row['longitude']]).add_to(marker_cluster)

# Function to create maps for a day
def create_maps_for_day(day, max_density):
    time_ranges = [
        (2, 8, f"map_{day}_2_to_8.html"),
        (8, 14, f"map_{day}_8_to_14.html"),
        (14, 20, f"map_{day}_14_to_20.html"),
        (20, 24, f"map_{day}_20_to_24.html")
    ]
    
    for start_hour, end_hour, output_filename in time_ranges:
        # Special handling for 20-24 range
        if start_hour == 20 and day != 20:  # Don't include next day for day 20
            df = load_data_for_time_range(day, start_hour, end_hour, next_day_data=True)
        else:
            df = load_data_for_time_range(day, start_hour, end_hour)
        
        # Filter data to the geographical bounds
        df = df[(df['latitude'] >= LAT_MIN) & (df['latitude'] <= LAT_MAX) &
                (df['longitude'] >= LON_MIN) & (df['longitude'] <= LON_MAX)]
        
        # Create the map
        folium_map = create_map()
        
        # Add heatmap and clustering
        add_heatmap(folium_map, df, max_density)
        add_marker_clustering(folium_map, df)
        
        # Save the map
        folium_map.save(output_filename)
        print(f"Map saved as {output_filename}")

def main():
    days = range(14, 21)  # Days 14 to 20
    
    # First pass: Calculate maximum density across all maps for absolute scaling
    max_density = 0
    for day in days:
        time_ranges = [(2, 8), (8, 14), (14, 20), (20, 24)]
        for start_hour, end_hour in time_ranges:
            if start_hour == 20 and day != 20:
                df = load_data_for_time_range(day, start_hour, end_hour, next_day_data=True)
            else:
                df = load_data_for_time_range(day, start_hour, end_hour)
            
            df = df[(df['latitude'] >= LAT_MIN) & (df['latitude'] <= LAT_MAX) &
                   (df['longitude'] >= LON_MIN) & (df['longitude'] <= LON_MAX)]
            
            # Get the heat data with temporary weights (using a dummy max_density)
            density = calculate_density(df, max_density if max_density > 0 else 1)  # Avoid division by 0
            if density:  # If there's data
                # Extract weights (third element of each tuple) and find the max
                weights = [point[2] for point in density]
                local_max_density = max(weights) * max_density if max_density > 0 else max(weights)
                max_density = min(max_density, local_max_density)
            print("Calculated density for day", day, "hour", start_hour)
    
    # Scale to 30,000 as maximum
    max_density = max(max_density, 30000)
    print(f"Global max_density set to: {max_density}")
    
    # Second pass: Create maps with absolute scaling
    for day in days:
        create_maps_for_day(day, max_density)
        print("Completed maps for day", day)

if __name__ == "__main__":
    main()