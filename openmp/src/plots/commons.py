import polars as pl
import matplotlib.pyplot as plt
import pathlib as path

pl.Config.set_tbl_rows(100)
pl.Config.set_tbl_cols(20)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

COL_NAMES = ['total', 'draw', 'scatter', 'sort', 'gather']

LARGE_MARK_SIZE = 100
SMALL_MARK_SIZE = 50
PRIMARY_MAKR_STYLE = 'o'
SECONDARY_MARK_STYLE = '^'
TIME_SCALE_FACTOR = 1e6


def resolve_date_from_path(path: path.Path) -> str:
    return '-'.join(path.stem.split('-')[2:])

