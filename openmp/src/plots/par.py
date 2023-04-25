import pathlib as path
import polars as pl
import numpy as np
import matplotlib.pyplot as plt
from .commons import COL_NAMES, TIME_SCALE_FACTOR, PRIMARY_MAKR_STYLE, SECONDARY_MARK_STYLE
from .commons import resolve_date_from_path


def process_par_exp(data_path: path.Path, plot_dir: path.Path):
    print(f'process_par_exp: {data_path}, {plot_dir}', end=' ')
    data_file_date = resolve_date_from_path(data_path)
    print(f'resolved date: {data_file_date}')

    data_df = (
        pl.scan_csv(data_path, has_header=True)
        .drop(['arrsize'])
        .select([pl.col(col_name) / TIME_SCALE_FACTOR for col_name in COL_NAMES]
                + [pl.exclude(COL_NAMES)])
        .collect()
    )

    bucket_size = data_df.get_column('bsize').head(1)[0]

    data_df = (
        data_df
        .lazy()
        .sort(['nthreads', 'sid'])
        .groupby(['nthreads'])
        .agg([pl.col(col_name).mean().alias(f'{col_name}_mean') for col_name in COL_NAMES]
             + [pl.col(col_name).std().alias(f'{col_name}_std') for col_name in COL_NAMES])
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

    for col_name in COL_NAMES[1:]:
        plot_par.errorbar(x_data, data_df.get_column(f'{col_name}_mean'), yerr=data_df.get_column(f'{col_name}_std'),
                          marker=SECONDARY_MARK_STYLE, linestyle=':', label=f'{col_name}')

    plot_par.set(
        title=f"Dane: 256MB ~ 33e6 elem., kubełek: {bucket_size}",
        xlabel="Liczba wątków",
        ylabel="Czas wykonania [s]")
    plot_par.grid()
    plot_par.legend()
    fig_par.tight_layout()
    fig_par.savefig(plot_dir.joinpath(f'par-time-256-{bucket_size}-{data_file_date}.png'))
    plt.close(fig_par)

    fig_bar, plot_bar = plt.subplots(nrows=1, ncols=1)

    y_offset = np.zeros(len(x_data))
    for col_name in COL_NAMES[1:]:
        data_series = data_df.get_column(f'{col_name}_mean')
        plt.bar(x_data, data_series, 0.4, bottom=y_offset, label=f'{col_name}')
        y_offset = y_offset + data_series

    plot_bar.set(
        title=f"Dane: 256MB ~ 33e6 elem., kubełek: {bucket_size}",
        xlabel="Liczba wątków",
        ylabel="Czas wykonania [s]")
    plot_bar.grid()
    plot_bar.legend()
    fig_bar.tight_layout()
    fig_bar.savefig(plot_dir.joinpath(
        f'par-bar-time-256-{bucket_size}-{data_file_date}.png'))
    plt.close(fig_bar)

    fig_sp, plot_sp = plt.subplots(nrows=1, ncols=1)

    y_data = total_mean_0 / y_data
    y_expected_sp = [x for x in x_data]

    plot_sp.plot(x_data, y_data, marker=PRIMARY_MAKR_STYLE,
                 linestyle='--', label='Średnia')
    plot_sp.plot(x_data, y_expected_sp, linestyle='dashdot', label='$y = x$')

    for col_name in COL_NAMES[1:]:
        data_series = data_df.get_column(f'{col_name}_mean')
        plot_sp.plot(x_data, data_series.head(1) / data_series, marker=SECONDARY_MARK_STYLE,
                     linestyle=':', label=f'{col_name}')

    plot_sp.set(
        title=f"Dane: 256MB ~ 33e6 elem., kubełek: {bucket_size}",
        xlabel="Liczba wątków",
        ylabel="Przyśpieszenie")
    plot_sp.grid()
    plot_sp.legend()
    fig_sp.tight_layout()
    fig_sp.savefig(plot_dir.joinpath(f'par-sp-256-{bucket_size}-{data_file_date}.png'))
    plt.close(fig_sp)

