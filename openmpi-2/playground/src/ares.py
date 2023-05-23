import pathlib as path
import polars as pl
import matplotlib.pyplot as plt
import numpy as np

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
# plt.rcParams['figure.figsize'] = (32, 18)
plt.rcParams['figure.figsize'] = (16, 9)
largemarkersize=100
smallmarkersize=50
primarymarkerstyle='o'
secondarymarkerstyle='^'

plotdir = path.Path('data', 'plots')
assert plotdir.is_dir(), "[ares] Plotdir exists"

exctx = "ares"
# exctx = "vcluster"

if exctx == 'ares':
    datadir = path.Path('data', 'ares')
    datafile = datadir.joinpath('ares-final-23-03-21-06-43.csv')
else:
    datadir = path.Path('data', 'vcluster')
    datafile = datadir.joinpath('vcluster-final-23-03-21-09-29.csv')


# vCluster
assert datadir.is_dir(), f"[{exctx}] Datadir exists"
assert datafile.is_file(), f"[{exctx}] main data file exists"


data_raw = pl.read_csv(datafile, has_header=True)
data_raw = (
    data_raw.replace('time', data_raw.get_column('time') / 1e6) # conversion from microseconds
    .drop('avg_pi') # don't need this column, as long as it looks ok -- it does!
    .rename({"proc_count": "procs", "total_point_count": "tpts", "point_count": "pts"})
)

data_s, data_w = data_raw.partition_by('type')

# Aggregated by series id
dtime_s = (
    data_s
    .groupby(['procs', 'tpts']) # key is series x procs x tpts
    .agg([pl.mean('time'), pl.std('time').alias('time_std')])
    .sort(pl.col(['procs', 'tpts']))
)

dtime_w = (
    data_w
    .groupby(['procs', 'pts'])
    .agg([pl.mean('time'), pl.std('time').alias('time_std')])
    .sort(pl.col(['procs', 'pts']))
)

point_counts = dtime_w.get_column('pts').unique().sort()
series = data_w.get_column('series').unique().sort()
xdata_time = dtime_w.get_column('procs').unique().sort()
xdata = xdata_time.clone().tail(-1)
xdata_bounds = xdata.head(1), xdata.tail(1)

dspeedup_s = (
    data_s
    .groupby(['procs', 'tpts'])
    .agg([pl.mean('time')])
    .with_columns((1 / pl.col('time')).alias('speedup'))
    .sort(pl.col(['procs', 'tpts']))
)

t1s = dspeedup_s.filter(pl.col('procs') == 1).get_column('time').sort()

for i, points in enumerate(point_counts):
    fig, [[plot_time, plot_sd], [plot_eff, plot_sf]] = plt.subplots(nrows=2, ncols=2)
    # fig_time, plot_time = plt.subplots()
    # fig_sd, plot_sd = plt.subplots()
    # fig_eff, plot_eff = plt.subplots()
    # fig_sf, plot_sf = plt.subplots()

    data_time = dtime_s.filter(pl.col('tpts') == points)
    plot_time.errorbar(xdata_time, data_time.get_column('time'), data_time.get_column('time_std'), linestyle=':')
    plot_time.scatter(xdata_time, data_time.get_column('time'), label='Średnia')
    plot_time.plot(xdata_time, data_time.get_column('time').max() / xdata_time, linestyle='--', label='$y=t_1 / x$')

    data_sd = dspeedup_s.filter(pl.col('tpts') == points).get_column('speedup').tail(-1) * t1s[i]
    plot_sd.scatter(xdata, data_sd, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sd.plot(xdata, data_sd, linestyle=':')
    plot_sd.plot(xdata_bounds, xdata_bounds, linestyle='--', label="y=x")

    # Serial Fraction
    data_sf = ((1 / data_sd) - (1 / xdata)) / (1 - 1 / xdata)
    plot_sf.scatter(xdata, data_sf, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sf.plot(xdata, data_sf, linestyle=':')
    plot_sf.plot([xdata.head(1), xdata.tail(1)], [0, 0], linestyle='--', label="y=0")

    # Efficiency
    data_eff = data_sd / xdata
    plot_eff.scatter(xdata, data_eff, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_eff.plot(xdata, data_eff, linestyle=':')
    plot_eff.plot(xdata_bounds, [1, 1], linestyle='--', label='y=x')

    for sid in series:
        data_sd = (
            data_s
            .filter((pl.col('series') == sid) & (pl.col('tpts') == points))
            .sort(pl.col('procs'))
        )
        ydata = data_sd.get_column('time')[0] / data_sd.get_column('time').tail(-1)
        plot_sd.scatter(xdata, ydata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_eff.scatter(xdata, ydata / xdata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_sf.scatter(xdata, ((1 / ydata) - (1 / xdata)) / (1 - 1 / xdata), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

        data_time = data_sd
        plot_time.scatter(xdata_time, data_time.get_column('time'), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

    plot_sd.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie sline)", xlabel="Liczba procesów", ylabel="Przyśpieszenie")
    plot_sd.grid()
    plot_sd.legend()

    plot_eff.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Efektywność")
    plot_eff.grid()
    plot_eff.legend()

    plot_sf.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Serial Fraction")
    plot_sf.grid()
    plot_sf.legend()

    plot_time.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Czas [s]")
    plot_time.grid()
    plot_time.legend()

    # fig_sd.savefig(plotdir.joinpath(f'{exctx}-speedup-s-{points:.0e}.png'))
    # fig_eff.savefig(plotdir.joinpath(f'{exctx}-eff-s-{points:.0e}.png'))
    # fig_sf.savefig(plotdir.joinpath(f'{exctx}-sf-s-{points:.0e}.png'))
    # fig_time.savefig(plotdir.joinpath(f'{exctx}-time-s-{points:.0e}.png'))
    fig.tight_layout()
    fig.savefig(plotdir.joinpath(f'{exctx}-combined-s-{points:.0e}.png'))


dspeedup_w = (
    data_w
    .groupby(['procs', 'pts'])
    .agg([pl.mean('time')])
    .with_columns((1 / pl.col('time') * pl.col('procs')).alias('speedup')) # ACHTUNG - scaling
    .sort(pl.col(['procs', 'pts']))
)

t1s = dspeedup_w.filter(pl.col('procs') == 1).get_column('time').sort()
for i, points in enumerate(point_counts):
    fig, [[plot_time, plot_sd], [plot_eff, plot_sf]] = plt.subplots(nrows=2, ncols=2)
    # fig_time, plot_time = plt.subplots()
    # fig_sd, plot_sd = plt.subplots()
    # fig_eff, plot_eff = plt.subplots()
    # fig_sf, plot_sf = plt.subplots()

    data_time = dtime_w.filter(pl.col('pts') == points)
    plot_time.errorbar(xdata_time, data_time.get_column('time'), data_time.get_column('time_std'), linestyle=':')
    plot_time.scatter(xdata_time, data_time.get_column('time'), label='Średnia')
    plot_time.plot(xdata_time, [data_time.get_column('time').max() for _ in range(len(xdata_time))], linestyle='--', label='$y=t_1$')

    data_sd = dspeedup_w.filter(pl.col('pts') == points).get_column('speedup').tail(-1) * t1s[i]
    plot_sd.scatter(xdata, data_sd, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sd.plot(xdata, data_sd, linestyle=':')
    plot_sd.plot([2, 12], [2, 12], linestyle='--', label="y=x")

    # Serial Fraction
    data_sf = ((1 / data_sd) - (1 / xdata)) / (1 - 1 / xdata)
    plot_sf.scatter(xdata, data_sf, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sf.plot(xdata, data_sf, linestyle=':')
    plot_sf.plot([xdata.head(1), xdata.tail(1)], [0, 0], linestyle='--', label="y=0")

    # Efficiency
    data_eff = data_sd / xdata
    plot_eff.scatter(xdata, data_eff, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_eff.plot(xdata, data_eff, markersize=smallmarkersize, linestyle=':')
    plot_eff.plot([2, 12], [1, 1], linestyle='--', label='y=x')

    for sid in series:
        data_sd = (
            data_w
            .filter((pl.col('series') == sid) & (pl.col('pts') == points))
            .sort(pl.col('procs'))
        )
        ydata = (data_sd.get_column('time')[0] * data_sd.get_column('procs') / data_sd.get_column('time')).tail(-1)
        plot_sd.scatter(xdata, ydata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_eff.scatter(xdata, ydata / xdata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_sf.scatter(xdata, ((1 / ydata) - (1 / xdata)) / (1 - 1 / xdata), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria {sid}')
        plot_time.scatter(xdata_time, data_sd.get_column('time'), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

    plot_sd.set(title=f'{exctx}, {points:.0e} punktów na proces (skalowanie słabe)', xlabel="Liczba procesów", ylabel="Przyśpieszenie")
    plot_sd.grid()
    plot_sd.legend()

    plot_eff.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Efektywność")
    plot_eff.grid()
    plot_eff.legend()

    plot_sf.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Serial Fraction")
    plot_sf.grid()
    plot_sf.legend()
    plot_time.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Czas [s]")
    plot_time.grid()
    plot_time.legend()

    # fig_sd.savefig(plotdir.joinpath(f'{exctx}-speedup-w-{points:.0e}.png'))
    # fig_eff.savefig(plotdir.joinpath(f'{exctx}-eff-w-{points:.0e}.png'))
    # fig_sf.savefig(plotdir.joinpath(f'{exctx}-sf-w-{points:.0e}.png'))
    # fig_time.savefig(plotdir.joinpath(f'{exctx}-time-w-{points:.0e}.png'))
    fig.tight_layout()
    fig.savefig(plotdir.joinpath(f'{exctx}-combined-w-{points:.0e}.png'))

plt.show()
plt.close()

exit(0)


if __name__ == "__main__":
    print(f'[{exctx}] Data files: {datafiles}')
