import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt

# Read in the cmap and mmap text files as DataFrames with space delimiter
# Use `dtype=float` initially to allow for missing values (NaN)
cmap = pd.read_csv('cmap.txt', delimiter=' ', header=None, dtype=float)
mmap = pd.read_csv('mmap.txt', delimiter=' ', header=None, dtype=float)

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
ax[0].set_title('cmap')
fig.colorbar(cmap_plot, ax=ax[0])

# Plot the mmap DataFrame
mmap_plot = ax[1].imshow(mmap, cmap='viridis', aspect='auto')
ax[1].set_title('mmap')
fig.colorbar(mmap_plot, ax=ax[1])

plt.tight_layout()
plt.show()