import os
import pandas as pd

def read_sample(folder_path: str) -> pd.DataFrame:
    count = 1
    for file in os.listdir(folder_path):
        if file.endswith(".parquet"):
            filepath = os.path.join(folder_path, file)
            df = pd.read_parquet(filepath, engine="pyarrow")
            filename = filepath.split("/")[-1]
            csv_file = f'july_csv/{"data"+str(count)}.csv'
            if os.path.isfile(csv_file):
                os.remove(csv_file)
            df.to_csv(csv_file, index=False)
            count += 1

#call read_sample function on the given file by command line argument
if __name__ == "__main__":
    import sys
    folder_path = sys.argv[1]
    read_sample(folder_path)
