import pathlib as path
import polars as pl
import numpy as np
import matplotlib.pyplot as plt
from .commons import COL_NAMES, TIME_SCALE_FACTOR, PRIMARY_MAKR_STYLE, SECONDARY_MARK_STYLE
from .commons import resolve_date_from_path


def process_cmp(data_file_1: path.Path, data_file_2: path.Path, plot_dir: path.Path):
    print(f'process_cmp: {data_file_1}, {data_file_2}, {plot_dir}')
    data_file_date_1 = resolve_date_from_path(data_file_1)
    data_file_date_2 = resolve_date_from_path(data_file_2)

    data_df_1 = (
        pl.scan_csv(data_file_1)
        .drop(['arrsize'])
        .select([pl.col(col_name) / TIME_SCALE_FACTOR for col_name in COL_NAMES]
                + [pl.exclude(COL_NAMES)])
        .collect()
    )

    data_df_2 = (
        pl.scan_csv(data_file_2)
        .drop(['arrsize'])
        .select([pl.col(col_name) / TIME_SCALE_FACTOR for col_name in COL_NAMES]
                + [pl.exclude(COL_NAMES)])
        .collect()
    )

    bucket_size = data_df_1.get_column('bsize')[0]
    nthreads_min = np.min([
        data_df_1.get_column('nthreads').max(),
        data_df_2.get_column('nthreads').max()
    ])

    print('Determined nthreads_max to', nthreads_min)

    data_df_1 = (
        data_df_1.lazy()
        .sort(['nthreads', 'sid'])
        .filter(pl.col('nthreads') <= nthreads_min)
        .groupby(['nthreads'])
        .agg([pl.col(col_name).mean().alias(f'{col_name}_mean') for col_name in COL_NAMES]
             + [pl.col(col_name).std().alias(f'{col_name}_std') for col_name in COL_NAMES])
        .collect()
    )

    data_df_2 = (
        data_df_2.lazy()
        .sort(['nthreads', 'sid'])
        .filter(pl.col('nthreads') <= nthreads_min)
        .groupby(['nthreads'])
        .agg([pl.col(col_name).mean().alias(f'{col_name}_mean') for col_name in COL_NAMES]
             + [pl.col(col_name).std().alias(f'{col_name}_std') for col_name in COL_NAMES])
        .collect()
    )

    x_data = [n for n in range(1, nthreads_min + 1)]

    y_data_1 = data_df_1.get_column('total_mean')
    y_data_2 = data_df_2.get_column('total_mean')

    total_mean_0_1 = y_data_1.head(1)[0]
    total_mean_0_2 = y_data_2.head(1)[0]

    fig, ((plot_draw, plot_scatter), (plot_sort, plot_gather)) = plt.subplots(nrows=2, ncols=2)
    plot_map = {
        "draw": plot_draw,
        "scatter": plot_scatter,
        "sort": plot_sort,
        "gather": plot_gather
    }

    for col_name in COL_NAMES[1:]:
        plot_map[col_name].errorbar(
            x_data,
            data_df_1.get_column(f'{col_name}_mean'),
            yerr=data_df_1.get_column(f'{col_name}_std'),
            marker=PRIMARY_MAKR_STYLE, linestyle='--', label=f'{col_name} 1'
        )

        plot_map[col_name].errorbar(
            x_data,
            data_df_2.get_column(f'{col_name}_mean'),
            yerr=data_df_1.get_column(f'{col_name}_std'),
            marker=SECONDARY_MARK_STYLE, linestyle='--', label=f'{col_name} 2'
        )

        plot_map[col_name].set(
            title=f"{col_name}, 256MB ~ 33e6 elem., kubełek {bucket_size}",
            xlabel="Liczba wątków",
            ylabel="Czas wykonania [s]"
        )
        plot_map[col_name].grid()
        plot_map[col_name].legend()

    fig.suptitle('Porównanie czasu wykonania poszczególnych faz w wersjach 1 i 2 algorytmu')
    fig.tight_layout()
    fig.savefig(plot_dir.joinpath(f'cmp-time-{bucket_size}-{data_file_date_1}-{data_file_date_2}.png'))
    # plt.close(fig)

    fig, ((plot_draw, plot_scatter), (plot_sort, plot_gather)) = plt.subplots(nrows=2, ncols=2)

    plot_map = {
        "draw": plot_draw,
        "scatter": plot_scatter,
        "sort": plot_sort,
        "gather": plot_gather
    }

    for col_name in COL_NAMES[1:]:
        ds_1 = data_df_1.get_column(f'{col_name}_mean')
        ds_2 = data_df_2.get_column(f'{col_name}_mean')

        plot_map[col_name].plot(
            x_data,
            ds_1.head(1) / ds_1,
            marker=PRIMARY_MAKR_STYLE, linestyle='--',
            label=f'{col_name} 1'
        )

        plot_map[col_name].plot(
            x_data,
            ds_2.head(1) / ds_2,
            marker=PRIMARY_MAKR_STYLE, linestyle='--',
            label=f'{col_name} 2'
        )

        plot_map[col_name].set(
            title=f"{col_name}, 256MB ~ 33e6 elem., kubełek {bucket_size}",
            xlabel="Liczba wątków",
            ylabel="Przyśpieszenie"
        )
        plot_map[col_name].grid()
        plot_map[col_name].legend()

    fig.suptitle('Porównanie przyśpieszenia poszczególnych faz w wersjach 1 i 2 algorytmu')
    fig.tight_layout()
    fig.savefig(plot_dir.joinpath(f'cmp-sp-{bucket_size}-{data_file_date_1}-{data_file_date_2}.png'))
    # plt.close(fig)

