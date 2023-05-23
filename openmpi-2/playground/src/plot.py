import numpy as np
import matplotlib.pyplot as plt
import polars as pl
import ares
import vcluster as vcl
from pathlib import Path
from typing import Literal

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)

ExperimentType = Literal['strong'] | Literal['weak']
PlatformType = Literal['vcluster'] | Literal['ares']

plotdir = Path('data', 'plots')

# vcluster_datafile = vcl.datadir.joinpath('vcluster-final-23-03-20-11-49.csv')
# vcluster_datafile = vcl.datadir.joinpath('vcluster-final-23-03-20-10-02.csv')
# # vcluster_datafile = datadir.joinpath('vcluster-final-1e8-23-03-20-15-16.csv')
# # vcluster_datafile = datadir.joinpath('vcluster-final-23-03-20-20-18.csv')

# # Extracting & preparing data
# vcluster_data = pl.read_csv(vcluster_datafile)

# df_main_agg = (
#     vcluster_data
#     .groupby(['type', 'proc_count'])
#     .agg([pl.mean('time') / 1e6, pl.std('time').alias('time_std') / 1e6])
#     .sort(pl.col(['type', 'proc_count']))
# )

# # Speedup
# t_1_strong = df_main_agg.filter(pl.col('type') == 'strong').get_column('time')[0]
# t_1_weak = df_main_agg.filter(pl.col('type') == 'weak').get_column('time')[0]

# df_strong_agg = (
#     df_main_agg
#     .filter(pl.col('type') == 'strong')
#     .filter(pl.col('proc_count') > 1)
#     .with_columns((t_1_strong / pl.col('time')).alias('speedup'))
#     .with_columns((pl.col('speedup') / pl.col('proc_count')).alias('eff'))
#     .with_columns(((1.0 / pl.col('speedup') - 1 / pl.col('proc_count')) / (1 - 1 / pl.col('proc_count'))).alias('sf'))
# )

# df_weak_agg = (
#     df_main_agg
#     .filter(pl.col('type') == 'weak')
#     .with_columns((t_1_weak * pl.col('proc_count') / pl.col('time')).alias('speedup'))
#     .filter(pl.col('proc_count') > 1)
#     .with_columns((pl.col('speedup') / pl.col('proc_count')).alias('eff'))
#     .with_columns(((1.0 / pl.col('speedup') - 1 / pl.col('proc_count')) / (1 - 1 / pl.col('proc_count'))).alias('sf'))
# )

# # Plotting

# scaling_types = ['weak', 'strong']

# # Time
# _, plot_weak = plt.subplots()
# _, plot_strong = plt.subplots()
# xdata = df_main_agg.get_column('proc_count').unique().sort()
# # print(xdata)

# for plot, xtype in [(plot_weak, 'weak'), (plot_strong, 'strong')]:
#     df_plot = df_main_agg.filter(pl.col('type') == xtype)
#     plot.scatter(xdata, df_plot.get_column('time'), label='Średnia')
#     plot.errorbar(xdata, df_plot.get_column('time'), yerr=df_plot.get_column('time_std'), linestyle=':')
#     for series in range(1, 4):
#         data = vcluster_data.filter(pl.col('series') == series).filter(pl.col('type') == xtype).sort(pl.col('proc_count'))
#         plot.scatter(xdata, data.get_column('time') / 1e6, label=f"Seria: {series}")

# plot_weak.set(title="vCluster, 1e9 punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Czas [s]")
# plot_strong.set(title="vCluster, 1e9 punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Czas [s]")

# plot_weak.legend()
# plot_strong.legend()


# # Speedup
# _, plot_weak = plt.subplots()
# _, plot_strong = plt.subplots()
# xdata = df_strong_agg.get_column('proc_count').unique().sort()
# print(xdata)

# for plot, xtype in zip([plot_weak, plot_strong], scaling_types):
#     df = df_weak_agg if xtype == 'weak' else df_strong_agg
#     plot.scatter(xdata, df.get_column('speedup'))
#     # plot.plot

# plot_weak.legend()
# plot_strong.legend()


for points in ares.point_counts:
    _, plot_w = plt.subplots()
    xdata = ares.timedf_w.get_column('proc_count').unique()
    data = ares.timedf_w.filter(pl.col('point_count') == points)
    plot_w.scatter(xdata, data.get_column('time'))
    plot_w.errorbar(xdata, data.get_column('time'), data.get_column('time_std'))
    plot_w.set(title=f"Ares, {points:e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Czas [s]")
    plot_w.grid()
    plot_w.legend()

    _, plot_s = plt.subplots()
    # data = ares.timedf_s.filter




plt.tight_layout()
plt.show()
