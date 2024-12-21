import os
import pandas as pd

def read_sample(filepath: str) -> pd.DataFrame:
    df = pd.read_parquet(filepath, engine="pyarrow")
    filename = filepath.split("/")[-1]
    csv_file = f'sample_data_csv/{filename}.csv'
    if os.path.isfile(csv_file):
        os.remove(csv_file)
    df.to_csv(csv_file, index=False) 
    return df

df = read_sample("sample_data_raw/part-00000-1489667c-4e58-4dfa-a7e3-97651d708182-c000.snappy.parquet")
print(df.head(5))