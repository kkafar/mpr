import pathlib as path
import polars as pl
import matplotlib.pyplot as plt

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

large_mark_size = 100
small_mark_size = 50
primary_mark_style = 'o'
secondary_mark_style = '^'

plot_dir = path.Path('data', 'plots')
if not plot_dir.is_dir():
    plot_dir.mkdir(parents=True, exist_ok=True)

assert plot_dir.is_dir(), "Plot directory exists"

data_file = path.Path('data-arch',
                      'ares',
                      'final-sync-2023-04-16-17-31-34.csv')
assert data_file.is_file(), "Data file exists"

data_raw_df = pl.scan_csv(data_file, has_header=True).drop(['arrsize', 'nthreads']).select([
    pl.col('total') / 1e6,
    pl.exclude('total')
])

data_df = (
        data_raw_df.select([
            pl.all(),
            pl.col('total').mean().over('bsize').alias('total_mean'),
            pl.col('total').std().over('bsize').alias('total_std')
        ])
        .sort(['bsize', 'sid'])
        .collect())

fig, plot = plt.subplots(nrows=1, ncols=1)

series_ids = data_df.get_column('sid').unique().sort()
x_data = data_df.get_column('bsize').unique().sort()

y_data = data_df.select(pl.col('total_mean').unique().over('bsize')).to_series()
y_error = data_df.select(pl.col('total_std').unique().over('bsize')).to_series()


print(x_data.shape, y_data.shape, y_error.shape)

plot.errorbar(x_data, y_data, yerr=y_error,
              marker=primary_mark_style, linestyle=':', label='Średnia')

for series_id in series_ids:
    plot.scatter(x_data, data_df.filter(pl.col('sid') == series_id).sort('bsize').to_series(),
                 marker=secondary_mark_style, label=f'Seria {series_id}')

plot.set(
        title="Czas wykonania alg. sekwencyjnego a rozmiar kubełków, dane: 512MB",
        xlabel="Rozmiar kubełka",
        ylabel="Czas wykonania [s]")


plot.grid()
plot.legend()
fig.tight_layout()

plt.show()
