import polars as pl
import matplotlib.pyplot as plt
# import sys
import pathlib as path

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

LARGE_MARK_SIZE = 100
SMALL_MARK_SIZE = 50
PRIMARY_MARK_STYLE = 'o'
SECONDARY_MARK_STYLE = '^'

col_names = ['time', 'mode', 'size', 'cores']
mode_names = ['ec2large', 'xlarge16', 'xlarge32',
              'xlarge48', 'large16', 'large32', 'large48']

labels = {
    'ec2large': 'seq 2 cores',
    'xlarge16': 'MR 4 x 4 cores',
    'xlarge32': 'MP 8 x 4 cores',
    'xlarge48': 'MP 12 x 4 cores',
    'large16': 'MP 8 x 2 cores',
    'large32': 'MP 16 x 2 cores',
    'large48': 'MP 24 x 2 cores'
}

results_final_dir = path.Path('results-final')
assert results_final_dir.is_dir()

data_file_path = results_final_dir.joinpath('result-23-05-22-17-29-09.txt')
assert data_file_path.is_file()

plot_dir = path.Path('plots')
assert plot_dir.is_dir()

raw_df = pl.scan_csv(data_file_path, has_header=True)

data_df_agg = (
    raw_df
    .lazy()
    .groupby(['size', 'mode', 'cores'])
    .agg([pl.col('time').mean().alias('time_mean'),
          pl.col('time').std().alias('time_std')])
    .sort(['size', 'mode']).collect()
)

data_df = (
    raw_df
    .lazy()
    .sort(['size', 'mode', 'cores'])
    .with_columns([
        pl.col('time').mean().over(['size', 'mode']).alias('time_mean'),
        pl.col('time').std().over(['size', 'mode']).alias('time_std')
    ]).collect()
)

print(data_df)
print(data_df_agg)

# Time(size)
x_data = data_df.get_column('size').unique().sort()
# ref_time = data_df
fig, plot = plt.subplots(nrows=1, ncols=1)
# fig_sp, plot_sp = plt.subplots(nrows=1, ncols=1)
for mode in mode_names:
    y_df = data_df_agg.filter(pl.col('mode') == mode).sort('size')
    y_data = y_df.get_column('time_mean')
    y_err = y_df.get_column('time_std')

    plot.errorbar(x_data, y_data, yerr=y_err, marker=SECONDARY_MARK_STYLE,
                  linestyle='--', label=labels[mode])


plot.set(
    title='Zależność czasu wykonania od rozmiaru problemu (różne konfiguracje)',
    xlabel='Rozmiar problemu [GB]',
    ylabel='Czas wykonania [s]'
)

# plot.set_xticks(ticks=x_data.clone())
plot.grid()
plot.legend()
fig.tight_layout()
fig.savefig(plot_dir.joinpath('time-size.png'))

# plot_sp.grid()
# plot_sp.legend()
# fig_sp.tight_layout()
# fig_sp.savefig(plot_dir.joinpath('sp-size.png'))


# Time(cores) & Speedup(cores)
x_data = data_df.get_column('cores').unique().sort().tail(-1)
problem_sizes = data_df_agg.get_column('size').unique().sort()

for psize in problem_sizes:
    fig, plot = plt.subplots(nrows=1, ncols=1)
    fig_sp, plot_sp = plt.subplots(nrows=1, ncols=1)

    print('ref_time')
    print(data_df_agg.filter((pl.col('mode') == mode_names[0]) & (
        pl.col('size') == psize)).get_column('time_mean'))
    ref_time = data_df_agg.filter((pl.col('mode') == mode_names[0]) & (
        pl.col('size') == psize)).get_column('time_mean')[0]
    tmp_df = data_df_agg.filter(pl.col('size') == psize)
    # xlarge
    y_df = tmp_df.filter(pl.col('mode').is_in(mode_names[1:4])).sort('cores')

    y_data = y_df.get_column('time_mean')
    y_err = y_df.get_column('time_std')
    print(x_data.shape, y_data.shape)
    plot.errorbar(x_data, y_data, yerr=y_err, marker=SECONDARY_MARK_STYLE,
                  linestyle='--', label=mode_names[1][:-2])

    y_sp_data = ref_time / y_data
    plot_sp.plot(x_data, y_sp_data, marker=PRIMARY_MARK_STYLE,
                 label=mode_names[1][:-2], linestyle='--')

    # large
    y_df = tmp_df.filter(pl.col('mode').is_in(mode_names[4:])).sort('cores')
    y_data = y_df.get_column('time_mean')
    y_err = y_df.get_column('time_std')
    plot.errorbar(x_data, y_data, yerr=y_err, marker=SECONDARY_MARK_STYLE,
                  linestyle='--', label=mode_names[-1][:-2])

    plot.plot(x_data, [ref_time for _ in range(len(x_data))], linestyle='dashdot', label='seq')

    y_sp_data = ref_time / y_data
    plot_sp.plot(x_data, y_sp_data, marker=PRIMARY_MARK_STYLE,
                 label=mode_names[-1][:-2], linestyle='--')

    # plot_sp.plot(x_data, [x for x in x_data], linestyle='dashdot', label='$y = x$')

    plot.set(
        title=f'Zależność czasu wykonania od liczby rdzeni (dane: {psize}GB)',
        xlabel='Liczba rdzeni',
        ylabel='Czas wykonania [s]'
    )

    plot.set_xticks(ticks=x_data)
    plot.grid()
    plot.legend()
    fig.tight_layout()
    fig.savefig(plot_dir.joinpath(f'time-cores-psize-{psize}.png'))


    plot_sp.set(
        title=f'Przyśpieszenie od liczby rdzeni (dane: {psize}GB)',
        xlabel='Liczba rdzeni',
        ylabel='Przyśpieszenie'
    )

    plot_sp.set_xticks(ticks=x_data)
    plot_sp.grid()
    plot_sp.legend()
    fig_sp.tight_layout()
    fig_sp.savefig(plot_dir.joinpath(f'sp-cores-psize-{psize}.png'))


plt.show()
