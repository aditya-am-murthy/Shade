import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import argparse

# Parse the command line arguments for the file name
data_path = 'condensed_data.csv'

# Read in the command line if there is a file name, and if there is, replace the default file name
args = argparse.ArgumentParser()
args.add_argument('--file_name', type=str, default=None)
args = args.parse_args()
if args.file_name:
    data_path = args.file_name

data = pd.read_csv(data_path, dtype={'cell ID': str})

#extracint the row and column numbers from the cell ID
data['row'] = data['cell ID'].apply(lambda x: int(str(x)[:2]))
data['col'] = data['cell ID'].apply(lambda x: int(str(x)[2:]))

# Create a 2D array of the cmap values
cmap = np.zeros((data['row'].max() + 1, data['col'].max() + 1))
for i in range(data.shape[0]):
    cmap[data.loc[i, 'row'], data.loc[i, 'col']] = data.loc[i, 'cmap val']

# Create a 2D array of the mmap values
mmap = np.zeros((data['row'].max() + 1, data['col'].max() + 1))
for i in range(data.shape[0]):
    mmap[data.loc[i, 'row'], data.loc[i, 'col']] = data.loc[i, 'mmap val']

# Plot the cmap values
plt.imshow(cmap, cmap='viridis')
plt.colorbar()
plt.title('Census map Values')
plt.show()

# Plot the mmap values
plt.imshow(mmap, cmap='viridis')
plt.colorbar()
plt.title('Mobile Phone Data map Values')
plt.show()

# graph the heatmaps ased on the latitude and longitude values
fig, ax = plt.subplots(1, 2, figsize=(12, 6))

# Plot the cmap DataFrame
cmap_plot = ax[0].scatter(data['longitude'], data['latitude'], c=data['cmap val'], cmap='viridis')
ax[0].set_title('Census map Values')
fig.colorbar(cmap_plot, ax=ax[0])

# Plot the mmap DataFrame
mmap_plot = ax[1].scatter(data['longitude'], data['latitude'], c=data['mmap val'], cmap='viridis')
ax[1].set_title('Mobile Phone Data map Values')
fig.colorbar(mmap_plot, ax=ax[1])

plt.show()
