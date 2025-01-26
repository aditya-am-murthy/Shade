import geopandas as gpd
import pandas as pd
import numpy as np
import re
import os

# Load the shapefile
#shapefile_path = '/Users/adityacode/Shade/census_data/tl_2024_06_bg.shp'
#gdf = gpd.read_file(shapefile_path)

#filter the data for countyfp = 037, 059, 111, 071, 065
#gdf_filtered = gdf[gdf['COUNTYFP'].isin(['037', '059', '111', '071', '065'])]
#print(gdf_filtered.head())

def convert_census_block_group_csv(census_file_path: str) -> dict:
    df = pd.read_csv(census_file_path)
    
    def regex_extractor(column_name: str) -> (str, int): # type: ignore
        pattern = r"Block Group (\d+); Census Tract (\d+); ([\w\s]+) County; ([\w\s]+)!!Estimate"
        match = re.match(pattern, column_name)
        
        if not match:
            return "", 0
        
        # Extract values from the FIRST ROW of the dataframe for this column
        value_str = str(df[column_name].iloc[0]).replace(",", "")  # <-- KEY FIX: Use .iloc[0]
        value = int(value_str) if value_str.isdigit() else 0
        
        # Create key: Tract + Block Group (e.g., "400101" for Tract 4001, Block Group 1)
        tract = match.group(2)
        block_group = match.group(1)
        key = f"{tract}{block_group.zfill(2)}"  # Pad block group to 3 digits
        
        return key, value
    
    hash_map = {}
    for column in df.columns[1::2]:  # Process every other column starting from index 1
        key, value = regex_extractor(column)
        if key:  # Only add valid entries
            hash_map[key] = value
    
    #export the data to a csv file
    if os.path.isfile('bg_to_pop.csv'):
        os.remove('bg_to_pop.csv')
    output_file = 'bg_to_pop.csv'
    df_output = pd.DataFrame(hash_map.items(), columns=['Tract_Block_Group', 'Population'])
    df_output.to_csv(output_file, index=False)

    return hash_map

def convert_shp_csv(shapefile_path: str) -> dict:
    shapes_df = gpd.read_file(shapefile_path)
    populations_df = pd.read_excel("/Users/adityacode/Shade/census_pop_data/pop.xls")

    shapes_df_filtered = shapes_df[shapes_df['COUNTYFP'].isin(['037', '059', '111', '071', '065'])]
    print(shapes_df.columns)
    print(shapes_df_filtered.head())

    shapes_df_filtered['TRACTCE'] = shapes_df_filtered['TRACTCE'].astype(str)
    populations_df['TRACT'] = populations_df['TRACT'].astype(str)
    
    #create a new df that has the Tract_Block_Group and Population columns from the populations_df, and the geometry column from the shapes_df_filtered. The merge should be so that all the TRACT in the populations_df match the TRACTCE in the shapes_df_filtered

    # Merge the two dataframes on the 'TRACTCE' column from shapes_df and 'TRACT' column from populations_df, keeping only the rows that match in both dataframes, and only keeping the columns 'TRACT' and 'POP100' from populations_df, and the 'geometry' column from shapes_df
    merged_df = pd.merge(shapes_df_filtered, populations_df, left_on='TRACTCE', right_on='TRACT', how='inner')[['TRACT', 'POP100', 'geometry']]

    #export the data to a csv file
    if os.path.isfile('tract_to_pop.csv'):
        os.remove('tract_to_pop.csv')
    output_file = 'tract_to_pop.csv'
    merged_df.to_csv(output_file, index=False)

    return merged_df

if __name__ == "__main__":
    shapefile_path = '/Users/adityacode/Shade/census_data/tl_2024_06_bg.shp'
    convert_shp_csv(shapefile_path)