import polars as pl
import matplotlib.pyplot as plt
import sys
import pathlib as path

results_final_dir = path.Path('results-final')
assert results_final_dir.is_dir()

data_file_path = results_final_dir.joinpath('result-23-05-22-17-29-09.txt')
assert data_file_path.is_file()

raw_df = pl.scan_csv(data_file_path, has_header=True)

print(raw_df.collect())

