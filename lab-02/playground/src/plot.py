import numpy as np
import matplotlib.pyplot as plt
import polars as pl
from pathlib import Path
from typing import Literal

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

ExperimentType = Literal['strong'] | Literal['weak']

def plot_time(df: pl.DataFrame, **plotmetadata):
    x = df.get_column('proc_count')
    y = df.get_column('time')
    yerr = df.get_column('time_std')

    _, plot = plt.subplots()
    plot.scatter(x, y)
    plot.errorbar(x, y, yerr=yerr, linestyle='--')
    plot.set(**plotmetadata)
    plot.grid(visible=True)

def plot_speedup(df: pl.DataFrame, exptype: ExperimentType):
    x = df.get_column('proc_count')
    y = df.get_column('speedup')

    _, plot = plt.subplots()
    plot.scatter(x, y)
    plot.plot(x, y, linestyle='--')
    plot.grid(visible=True)
    # plot.errorbar(x, y, yerr=yerr, linestyle='--')

def plot_eff(df: pl.DataFrame):
    x = df.get_column('proc_count')
    y = df.get_column('eff')

    _, plot = plt.subplots()
    plot.scatter(x, y)
    plot.plot(x, y, linestyle='--')
    plot.grid(visible=True)

def plot_sf(df: pl.DataFrame):
    x = df.get_column('proc_count')
    y = df.get_column('sf')

    _, plot = plt.subplots()
    plot.scatter(x, y)
    plot.plot(x, y, linestyle='--')
    plot.grid(visible=True)


datadir = Path('data')
vcluster_datafile = datadir.joinpath('vcluster-final-23-03-20-11-49.csv')

if not datadir.is_dir():
    print("Data directory does not exist")
    exit(1)

if not vcluster_datafile.is_file():
    print("vCluster data file does not exist")
    exit(1)

vcluster_data = pl.read_csv(vcluster_datafile)

df = (
    vcluster_data
    .groupby(['type', 'proc_count'])
    .agg([pl.mean('time') / 1e6, pl.std('time').alias('time_std') / 1e6])
    .sort(pl.col(['type', 'proc_count']))
)

print(df)

# przyśpieszenie
t_1_strong = df.filter(pl.col('type') == 'strong').get_column('time')[0]
t_1_weak = df.filter(pl.col('type') == 'weak').get_column('time')[0]

print(t_1_weak)

df_speedup_strong = (
    df
    .filter(pl.col('type') == 'strong')
    .filter(pl.col('proc_count') > 1)
    .with_columns((t_1_strong / pl.col('time')).alias('speedup'))
    .with_columns((pl.col('speedup') / pl.col('proc_count')).alias('eff'))
    .with_columns(((1.0 / pl.col('speedup') - 1 / pl.col('proc_count')) / (1 - 1 / pl.col('proc_count'))).alias('sf'))
)

df_speedup_weak = (
    df
    .filter(pl.col('type') == 'weak')
    .with_columns((t_1_weak * pl.col('proc_count') / pl.col('time')).alias('speedup'))
    .filter(pl.col('proc_count') > 1)
    .with_columns((pl.col('speedup') / pl.col('proc_count')).alias('eff'))
    .with_columns(((1.0 / pl.col('speedup') - 1 / pl.col('proc_count')) / (1 - 1 / pl.col('proc_count'))).alias('sf'))
)

print(df_speedup_strong)
print(df_speedup_weak)

plot_time(df.filter(pl.col('type') == 'strong'), title="vCluster, 1e9 punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Czas [s]")
plot_time(df.filter(pl.col('type') == 'weak'), title="vCluster, 1e9 punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Czas [s]")

# plot_speedup(df_speedup_strong, 'strong')
# plot_speedup(df_speedup_weak, 'weak')

# plot_eff(df_speedup_strong)
# plot_eff(df_speedup_weak)

# plot_sf(df_speedup_strong)
# plot_sf(df_speedup_weak)

plt.tight_layout()
plt.show()
