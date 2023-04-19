import pathlib as path
import matplotlib.pyplot as plt
import sys
from plots.par import process_par_exp
from plots.seq import processs_seq_exp


plot_dir = path.Path('plots', 'sort')
if not plot_dir.is_dir():
    plot_dir.mkdir(parents=True, exist_ok=True)

assert plot_dir.is_dir(), "Plot directory exists"


data_par_file = path.Path('data-arch',
                          'ares',
                          'final-async-2023-04-17-15-17-02.csv')

data_seq_file = path.Path('data-arch',
                          'ares',
                          # 'final-sync-2023-04-16-17-31-34.csv')
                          # 'final-sync-2023-04-17-03-11-38.csv')
                          # 'final-sync-2023-04-17-01-32-48.csv')
                          # 'final-sync-2023-04-17-03-11-38.csv')
                          'final-sync-2023-04-19-15-00-00.csv')

assert data_seq_file.is_file(), "Seq data file exists"
assert data_par_file.is_file(), "Par data file exists"

col_names = ['total', 'draw', 'scatter', 'sort', 'gather']

# Sequential experiment plots

processs_seq_exp(data_seq_file, plot_dir)
process_par_exp(data_par_file, plot_dir)


plt.show()

