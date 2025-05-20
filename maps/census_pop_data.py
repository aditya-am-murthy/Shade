from dotenv import load_dotenv
import os
import pandas as pd

def get_population_data():
    load_dotenv()  # Load environment variables from .env file
    api_key = os.getenv('CENSUS_API_KEY')  # Get the API key from environment variable
    if not api_key:
        raise ValueError("API key not found. Please set it in your .env file.")
    
    url = f'https://api.census.gov/data/2021/pep/population?get=DENSITY_2021,POP_2021,NAME,STATE&for=region:*&key={api_key}'
    df = pd.read_json(url)  # Read the JSON response into a DataFrame
    return df

# Call get_population_data function
if __name__ == "__main__":
    df = get_population_data()
    print(df)  # Optionally print the dataframe to check if it worked
