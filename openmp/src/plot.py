import pathlib as path
import polars as pl
import matplotlib.pyplot as plt
import numpy as np

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

large_mark_size = 100
small_mark_size = 50
primary_mark_style = 'o'
secondary_mark_style = '^'

TIME_SCALE_FACTOR = 1e6


plot_dir = path.Path('data', 'plots')
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
                          'final-sync-2023-04-17-01-32-48.csv')

assert data_seq_file.is_file(), "Seq data file exists"
assert data_par_file.is_file(), "Par data file exists"

data_raw_df = (
    pl.scan_csv(data_seq_file, has_header=True)
    .drop(['arrsize', 'nthreads'])
    .select([
        pl.col('total') / TIME_SCALE_FACTOR,
        pl.exclude('total')
    ])
)

data_df = (
    data_raw_df.select([
        pl.all(),
        pl.col('total').mean().over('bsize').alias('total_mean'),
        pl.col('total').std().over('bsize').alias('total_std')
    ])
    .sort(['bsize', 'sid'])
    .collect()
)

fig, plot = plt.subplots(nrows=1, ncols=1)

series_ids = data_df.get_column('sid').unique().sort()
x_data = data_df.get_column('bsize').unique().sort()

y_data = data_df.select(pl.col('total_mean').unique().over('bsize')).to_series()
y_error = data_df.select(pl.col('total_std').unique().over('bsize')).to_series()


plot.errorbar(x_data, y_data, yerr=y_error,
              marker=primary_mark_style, linestyle=':', label='Średnia')

for series_id in series_ids:
    plot.scatter(x_data, data_df.filter(pl.col('sid') == series_id).sort('bsize').to_series(),
                 marker=secondary_mark_style, label=f'Seria {series_id}')

plot.set(
    title="Czas wykonania alg. sekwencyjnego a rozmiar kubełków, dane: 256MB",
    xlabel="Rozmiar kubełka",
    ylabel="Czas wykonania [s]")
plot.grid()
plot.legend()
fig.tight_layout()
fig.savefig(plot_dir.joinpath('seq-256.png'))

col_names = ['total', 'draw', 'scatter', 'sort', 'gather']

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
                  marker=primary_mark_style, linestyle='--', label='Czas całkowity')
plot_par.plot(x_data, y_expected_1, linestyle='dashdot', label='$y = t_0 / x$')

for col_name in col_names[1:]:
    plot_par.errorbar(x_data, data_df.get_column(f'{col_name}_mean'), yerr=data_df.get_column(f'{col_name}_std'),
                      marker=secondary_mark_style, linestyle=':', label=f'{col_name}')

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

plot_sp.plot(x_data, y_data, marker=primary_mark_style, linestyle='--', label='Średnia')
plot_sp.plot(x_data, y_expected_sp, linestyle='dashdot', label='$y = x$')

for col_name in col_names[1:]:
    data_series = data_df.get_column(f'{col_name}_mean')
    plot_sp.plot(x_data, data_series.head(1) / data_series, marker=secondary_mark_style,
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

