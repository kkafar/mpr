import polars as pl
import matplotlib.pyplot as plt
import pathlib as plib

pl.Config.set_tbl_rows(200)
pl.Config.set_tbl_cols(50)
plt.rcParams['figure.figsize'] = (16, 9)

data_file_path = plib.Path('data', 'exp-2.csv')
plot_dir_path = plib.Path('plots')

assert data_file_path.is_file()
if not plot_dir_path.is_dir():
    plot_dir_path.mkdir(exist_ok=True, parents=True)

assert plot_dir_path.is_dir()


raw_df = pl.read_csv(data_file_path, has_header=True)

# print(raw_df)
# print(raw_df.filter(pl.col('type') == 'naive'))

# Single plot
# Arrsize on X axis and time on Y axis
# Series for each block size

data_df = (
    raw_df
    .lazy()
    .groupby(['type', 'msize', 'blocksize'])
    .agg([
        pl.col('time').mean().alias('time_avg'),
        pl.col('time').std().alias('time_std')
    ])
    .collect()
    .sort(['type', 'msize', 'blocksize'])
)

print(data_df)


x_data = data_df.get_column('msize').unique().sort()
block_sizes = data_df.get_column('blocksize').unique().sort()

fig, plot = plt.subplots(nrows=1, ncols=1)

for exp_type in ['naive', 'shared']:
    marker_type = 'o' if exp_type == 'naive' else '^'
    for block_size in block_sizes:
        plot_data = data_df.filter(pl.col('type') == exp_type).filter(
            pl.col('blocksize') == block_size)

        print("Data for", exp_type, block_size)
        print(plot_data)

        y_data = plot_data.get_column('time_avg')
        y_err = plot_data.get_column('time_std')
        plot.errorbar(x_data, y_data, yerr=y_err, marker=marker_type,
                      linestyle=':', label=f'Typ: {exp_type}, Rozmiar bloku: {block_size}')

plot.set(
    title='Czas wykonania w zależności od rozmiaru macierzy dla danej liczby wątków i liczby bloków. Liczba bloków to (<rozmiar tablicy> / <liczba wątków>)',
    xlabel='Rozmiar macierzy',
    ylabel='Czas wykonania [ms]'
)
plot.grid()
plot.legend()
fig.tight_layout()
fig.savefig(plot_dir_path.joinpath('exp-2-time-by-matrix-size-all-block-sizes.png'))


fig, plot = plt.subplots(nrows=1, ncols=1)
for exp_type in ['naive', 'shared']:
    marker_type = 'o' if exp_type == 'naive' else '^'
    for block_size in [4096]:
        plot_data = data_df.filter(pl.col('type') == exp_type).filter(
            pl.col('blocksize') == block_size)

        print("Data for", exp_type, block_size)
        print(plot_data)

        y_data = plot_data.get_column('time_avg')
        y_err = plot_data.get_column('time_std')
        plot.errorbar(x_data, y_data, yerr=y_err, marker=marker_type,
                      linestyle=':', label=f'Typ: {exp_type}, Rozmiar bloku: {block_size}')

plot.set(
    title='Czas wykonania w zależności od rozmiaru macierzy dla danej liczby wątków i liczby bloków. Liczba bloków to (<rozmiar tablicy> / <liczba wątków>)',
    xlabel='Rozmiar macierzy',
    ylabel='Czas wykonania [ms]'
)
plot.grid()
plot.legend()
fig.tight_layout()
fig.savefig(plot_dir_path.joinpath('exp-2-time-by-matrix-size-4096-block-size.png'))

plt.show()

