import pathlib as path
import polars as pl
import matplotlib.pyplot as plt

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)
largemarksize = 100
smallmarksize = 50
primarymarkstyle = 'o'
secondarymarkstyle = '^'

plotdir = path.Path('plots')
if not plotdir.is_dir():
    plotdir.mkdir(parents=True, exist_ok=True)

assert plotdir.is_dir(), "Plotdir exists"


exctx = 'vcluster'
datafile = path.Path('data-arch', 'draw-exp', 'vcluster-final-2023-04-04-10-10-55.csv')
# datafile = path.Path('data-arch', 'vcluster-final-2023-04-04-08-59-35.csv')
# datafile = path.Path('data-arch', 'vcluster-final-2023-04-04-07-35-36.csv')

# exctx='pc'
# datafile = path.Path('data-arch', 'final-2023-04-04-09-31-20.csv')

assert datafile.is_file(), "Datafile exists and is a file"

data_raw = pl.read_csv(datafile, has_header=True).sort(
    pl.col(['type', 'threads', 'chunk', 'size']))

# print(data_raw)

# primary key is:
# sid x type x threads x chunk x size

data_mean = (
    data_raw
    .groupby(['type', 'threads', 'chunk', 'size'])
    .agg([pl.mean('time'), pl.std('time').alias('time_std')])
    .sort(pl.col(['type', 'threads', 'chunk', 'size']))
)

# print(data_mean)

series_ids = data_raw.get_column('sid').unique().sort()
chunktypes = data_mean.get_column('chunk').unique().sort()
threads = data_mean.get_column('threads').unique().sort()
arrsizes = data_mean.get_column('size').unique().sort()
exptypes = data_mean.get_column('type').unique().sort()

# Plot time
for xtype in exptypes:
    for size in arrsizes:
        fig, (plot_time, plot_sp) = plt.subplots(nrows=1, ncols=2)

        data_time = data_mean.filter((pl.col('size') == size) & (pl.col('type') == xtype))
        data_time_ch_2 = data_time.filter(pl.col('chunk') == '2').sort(
            pl.col('threads')).select(pl.col('time'))
        data_time_ch_auto = data_time.filter(pl.col('chunk') == 'auto').sort(
            pl.col('threads')).select(pl.col('time'))
        # print(data_time_ch_2)
        plot_time.plot(threads, data_time_ch_2, label="Chunk: 2",
                       marker=primarymarkstyle, linestyle=':')
        plot_time.plot(threads, data_time_ch_auto, label="Chunk: auto",
                       marker=secondarymarkstyle, linestyle=':')

        strsize = int(size * 8 / 2 ** 20)
        plot_time.set(
            title=f"{exctx}, schedule: {xtype}, size: {strsize}MB",
            xlabel="Górny limit liczby wątków",
            ylabel="Czas [us]"
        )

        t1_ch_2 = data_time_ch_2.to_series()[0]
        t1_ch_auto = data_time_ch_auto.to_series()[0]
        data_speedup_ch_2 = data_time.filter(pl.col('chunk') == '2').sort(
            pl.col('threads')).select((t1_ch_2 / pl.col('time')).alias("speedup"))
        data_speedup_ch_auto = data_time.filter(pl.col('chunk') == 'auto').sort(
            pl.col('threads')).select((t1_ch_auto / pl.col('time')).alias("speedup"))

        plot_sp.plot(threads, data_speedup_ch_2, label="Chunk: 2",
                     marker=primarymarkstyle, linestyle=":")
        plot_sp.plot(threads, data_speedup_ch_auto, label="Chunk: auto",
                     marker=secondarymarkstyle, linestyle=":")

        plot_sp.set(
            title=f"{exctx}, schedule: {xtype}, size: {strsize}MB, skalowanie silne",
            xlabel="Górny limit liczby wątków",
            ylabel="Przyśpieszenie"
        )

        plot_time.grid()
        plot_time.legend()
        plot_sp.grid()
        plot_sp.legend()

        fig.tight_layout()
        fig.savefig(plotdir.joinpath(f'combined-{xtype}-{strsize}.png'))

plt.show()
plt.close()

