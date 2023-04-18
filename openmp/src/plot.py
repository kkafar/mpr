import pathlib as path
import polars as pl
import matplotlib.pyplot as plt
import numpy as np

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

LARGE_MARK_SIZE = 100
SMALL_MARK_SIZE = 50
PRIMARY_MAKR_STYLE = 'o'
SECONDARY_MARK_STYLE = '^'

TIME_SCALE_FACTOR = 1e6

plot_dir = path.Path('plots')
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
                          'final-sync-2023-04-17-03-11-38.csv')

assert data_seq_file.is_file(), "Seq data file exists"
assert data_par_file.is_file(), "Par data file exists"

col_names = ['total', 'draw', 'scatter', 'sort', 'gather']

# Sequential experiment plots

data_df = (
    pl.scan_csv(data_seq_file, has_header=True)
    .drop(['arrsize', 'nthreads'])
    .select([
        pl.col('total') / TIME_SCALE_FACTOR,
        pl.exclude('total')
    ])
    .sort(['bsize', 'sid'])
    .groupby(['bsize'])
    .agg(
        [
            pl.col('total').mean().alias('total_mean'),
            pl.col('total').std().alias('total_std')
        ]
    )
    .collect()
)

fig, plot = plt.subplots(nrows=1, ncols=1)

x_data = data_df.get_column('bsize')
y_data = data_df.get_column('total_mean')
y_error = data_df.get_column('total_std')

plot.errorbar(x_data, y_data, yerr=y_error,
              marker=PRIMARY_MAKR_STYLE, linestyle=':', label='Średnia')

plot.set(
    title="Czas wykonania alg. sekwencyjnego a rozmiar kubełków, dane: 256MB",
    xlabel="Rozmiar kubełka",
    ylabel="Czas wykonania [s]")
plot.grid()
plot.legend()
fig.tight_layout()
fig.savefig(plot_dir.joinpath('seq-256.png'))

# Parallel experiment plots

data_df = (
    pl.scan_csv(data_par_file, has_header=True)
    .drop(['arrsize', 'bsize'])
    .select([pl.col(col_name) / TIME_SCALE_FACTOR for col_name in col_names] + [pl.exclude(col_names)])
    .sort(['nthreads', 'sid'])
    .groupby(['nthreads'])
    .agg([pl.col(col_name).mean().alias(f'{col_name}_mean') for col_name in col_names]
         + [pl.col(col_name).std().alias(f'{col_name}_std') for col_name in col_names])
    .collect()
)

x_data = data_df.get_column('nthreads').sort()
y_data = data_df.get_column('total_mean')
total_mean_0 = y_data.head(1)
y_expected_1 = [total_mean_0 / t for t in x_data]

fig_par, plot_par = plt.subplots(nrows=1, ncols=1)

plot_par.errorbar(x_data, y_data, yerr=data_df.get_column('total_std'),
                  marker=PRIMARY_MAKR_STYLE, linestyle='--', label='Czas całkowity')
plot_par.plot(x_data, y_expected_1, linestyle='dashdot', label='$y = t_0 / x$')

for col_name in col_names[1:]:
    plot_par.errorbar(x_data, data_df.get_column(f'{col_name}_mean'), yerr=data_df.get_column(f'{col_name}_std'),
                      marker=SECONDARY_MARK_STYLE, linestyle=':', label=f'{col_name}')

plot_par.set(
    title="Dane: 256MB, kubełek: 500",
    xlabel="Liczba wątków",
    ylabel="Czas wykonania [s]")
plot_par.grid()
plot_par.legend()
fig_par.tight_layout()
fig_par.savefig(plot_dir.joinpath('par-time-256-500.png'))

fig_bar, plot_bar = plt.subplots(nrows=1, ncols=1)

y_offset = np.zeros(len(x_data))
for col_name in col_names[1:]:
    data_series = data_df.get_column(f'{col_name}_mean')
    plt.bar(x_data, data_series, 0.4, bottom=y_offset, label=f'{col_name}')
    y_offset = y_offset + data_series

plot_bar.set(
    title="Dane: 256MB, kubełek: 500",
    xlabel="Liczba wątków",
    ylabel="Czas wykonania [s]")
plot_bar.grid()
plot_bar.legend()
fig_bar.tight_layout()
fig_bar.savefig(plot_dir.joinpath('par-bar-time-256-500.png'))

fig_sp, plot_sp = plt.subplots(nrows=1, ncols=1)

y_data = total_mean_0 / y_data
y_expected_sp = [x for x in x_data]

plot_sp.plot(x_data, y_data, marker=PRIMARY_MAKR_STYLE, linestyle='--', label='Średnia')
plot_sp.plot(x_data, y_expected_sp, linestyle='dashdot', label='$y = x$')

for col_name in col_names[1:]:
    data_series = data_df.get_column(f'{col_name}_mean')
    plot_sp.plot(x_data, data_series.head(1) / data_series, marker=SECONDARY_MARK_STYLE,
                 linestyle=':', label=f'{col_name}')


plot_sp.set(
    title="Dane: 256MB, kubełek: 500",
    xlabel="Liczba wątków",
    ylabel="Przyśpieszenie")
plot_sp.grid()
plot_sp.legend()
fig_sp.tight_layout()
fig_sp.savefig(plot_dir.joinpath('par-sp-256-500.png'))

plt.show()

