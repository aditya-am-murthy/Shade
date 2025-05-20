import geopandas as gpd

# Load the shapefile
shapefile_path = '/Users/adityacode/Shade/census_data/tl_2024_06_bg.shp'
gdf = gpd.read_file(shapefile_path)

#filter the data for countyfp = 037, 059, 111, 071, 065
gdf_filtered = gdf[gdf['COUNTYFP'].isin(['037', '059', '111', '071', '065'])]
print(gdf_filtered.shape)