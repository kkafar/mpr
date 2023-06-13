import polars as pl
import matplotlib.pyplot as plt
import pathlib as plib

pl.Config.set_tbl_rows(200)
pl.Config.set_tbl_cols(50)
plt.rcParams['figure.figsize'] = (16, 9)

data_file_path = plib.Path('data', 'exp-with-threads.csv')
plot_dir_path = plib.Path('plots')

assert data_file_path.is_file()
if not plot_dir_path.is_dir():
    plot_dir_path.mkdir(exist_ok=True, parents=True)

assert plot_dir_path.is_dir()


raw_df = pl.read_csv(data_file_path, has_header=True)

# print(raw_df)

# Single plot
# Arrsize on X axis and time on Y axis
# Series for each block size

data_df = (
    raw_df
    .lazy()
    .groupby(['arrsize', 'nthreads', 'nblocks'])
    .agg([
        pl.col('time').mean().alias('time_avg'),
        pl.col('time').std().alias('time_std')
    ])
    .collect()
    .sort(['arrsize', 'nthreads'])
)

print(data_df)

x_data = data_df.get_column('arrsize').unique().sort()
thread_counts = data_df.get_column('nthreads').unique().sort()
block_sizes = data_df.get_column('nblocks')

fig, plot = plt.subplots(nrows=1, ncols=1)

for n_threads in thread_counts:
    plot_data = data_df.filter(pl.col('nthreads') == n_threads)
    y_data = plot_data.get_column('time_avg')
    y_err = plot_data.get_column('time_std')
    plot.errorbar(x_data, y_data, yerr=y_err, marker='o',
                  linestyle=':', label=f'Wątki w bloku: {n_threads}')

    plot.set(
        title='Czas wykonania w zależności od rozmiaru tablicy dla danej liczby wątków i liczby bloków. Liczba bloków to (<rozmiar tablicy> / <liczba wątków>)',
        xlabel='Rozmiar tablicy',
        ylabel='Czas wykonania [ms]'
    )
    plot.grid()
    plot.legend()
    fig.tight_layout()
    fig.savefig(plot_dir_path.joinpath('exp-1-time-by-arr-size.png'))


plt.show()

