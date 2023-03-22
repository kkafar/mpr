import polars as pl
import matplotlib.pyplot as plt
import pathlib as path

datadir = path.Path('data', 'vcluster')

assert datadir.is_dir(), "[vcluster] Datadir exists"

datafiles = [
    file for file in datadir.glob("vcluster-*")
]


if __name__ == "__main__":
    print(f'[vcluster] Data files: {datafiles}')
