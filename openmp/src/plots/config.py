import polars as pl
import matplotlib.pyplot as plt

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

COL_NAMES = ['total', 'draw', 'scatter', 'sort', 'gather']

LARGE_MARK_SIZE = 100
SMALL_MARK_SIZE = 50
PRIMARY_MAKR_STYLE = 'o'
SECONDARY_MARK_STYLE = '^'
TIME_SCALE_FACTOR = 1e6

