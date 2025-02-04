import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt

#argparse for the command line arguments
import argparse

#make the 1st argument the cmap file and the 2nd argument the mmap file, set them to cmap_file_path and mmap_file_path, and if they are not provided, set them to cmap.txt and mmap.txt
parser = argparse.ArgumentParser(description='Process cmap and mmap files.')
parser.add_argument('cmap_file_path', type=str, nargs='?', default='cmap.txt', help='Path to the cmap file')
parser.add_argument('mmap_file_path', type=str, nargs='?', default='mmap.txt', help='Path to the mmap file')
args = parser.parse_args()

#take the cmap_file_path and mmap_file_path arguments and set them to cmap_file_path and mmap_file_path
cmap_file_path = args.cmap_file_path
mmap_file_path = args.mmap_file_path

# Read in the cmap and mmap text files as DataFrames with space delimiter
# Use `dtype=float` initially to allow for missing values (NaN)
cmap = pd.read_csv(cmap_file_path, delimiter=' ', header=None, dtype=float)
mmap = pd.read_csv(mmap_file_path, delimiter=' ', header=None, dtype=float)

# Fill missing values (NaN) with 0 (or any other value you prefer)
cmap = cmap.fillna(0)
mmap = mmap.fillna(0)

# Convert the DataFrames to integers
cmap = cmap.astype(int)
mmap = mmap.astype(int)

# Print the updated DataFrames
print("Updated cmap:")
print(cmap)

print("\nUpdated mmap:")
print(mmap)

#create a new df, score that represents the element-wise division of mmap by cmap, but when cmap is 0, the score is NAN
score = mmap / cmap.replace(0, np.nan)

#print summary statistics of the score dataframe
print("\nSummary statistics of the score dataframe:")
print(score.describe())

#determine the variance of the score dataframe
variance = score.var()
print("\nVariance of the score dataframe:")
print(variance)

#determine the median of the variance of the score dataframe, ignoring variances of NaN
median_variance = variance.median(skipna=True)
print("\nMedian variance of the score dataframe:")
print(median_variance)


#plot a heat map of the cmap and mmap dataframes side by side
fig, ax = plt.subplots(1, 2, figsize=(12, 6))

# Plot the cmap DataFrame
cmap_plot = ax[0].imshow(cmap, cmap='viridis', aspect='auto')
ax[0].set_title(cmap_file_path)
fig.colorbar(cmap_plot, ax=ax[0])

# Plot the mmap DataFrame
mmap_plot = ax[1].imshow(mmap, cmap='viridis', aspect='auto')
ax[1].set_title(mmap_file_path)
fig.colorbar(mmap_plot, ax=ax[1])

#rotate both the plots 180 degrees
for a in ax:
    a.invert_yaxis()
    a.invert_xaxis()

#overlay the plots onto google maps
# Load the Google Maps image of Los Angeles
img = plt.imread("https://upload.wikimedia.org/wikipedia/commons/thumb/8/8c/Los_Angeles_%28California%29%2C_Angelino_Heights%2C_Echo_Park_Lake%2C_Downtown_L.A._%282013%29.jpg/1024px-Los_Angeles_%28California%29%2C_Angelino_Heights%2C_Echo_Park_Lake%2C_Downtown_L.A._%282013%29.jpg")

# Plot the Google Maps image
ax[0].imshow(img, extent=[-118.3, -118.15, 34.05, 34.1])
ax[1].imshow(img, extent=[-118.3, -118.15, 34.05, 34.1])

# Set the aspect of the plots to be equal
for a in ax:
    a.set_aspect('equal')

# Set the title of the figure
fig.suptitle('Cmap and Mmap Heatmaps Overlayed on Google Maps')

# Adjust the layout

plt.tight_layout()
plt.show()