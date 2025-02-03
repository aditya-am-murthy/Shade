# read in the space delimited data for the mmap.txt and cmap.txt and condense it such taht we make a csv file with the columns: cell ID, cmap val, mmap val
# the cell id is a four digit number 0000, such that the first two digits correpond to the row/line number, and the last two digits correspond to the column number

import csv

# read in the data from the mmap.txt file
mmap = []
with open('mmap.txt', 'r') as f:
    for line in f:
        mmap.append(line.split())
    
# read in the data from the cmap.txt file
cmap = []
with open('cmap.txt', 'r') as f:
    for line in f:
        cmap.append(line.split())

# reverse the cmap and mmap lists, and reverse all sublists in the lists
cmap = cmap[::-1]
mmap = mmap[::-1]
for i in range(len(cmap)):
    cmap[i] = cmap[i][::-1]
    mmap[i] = mmap[i][::-1]

# create a csv file with the columns: cell ID, cmap val, mmap val
with open('condensed_data.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['cell ID', 'cmap val', 'mmap val'])
    for i in range(len(mmap)):
        for j in range(len(mmap[0])):
            writer.writerow([str(i).zfill(2) + str(j).zfill(2), cmap[i][j], mmap[i][j]])
    #if the last row is empty, delete it
    if i == len(mmap) - 1:
        if mmap[i] == ['']:
            f.seek(0, 2)

print('done')