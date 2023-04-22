import pathlib as path
import polars as pl
import matplotlib.pyplot as plt
from .commons import TIME_SCALE_FACTOR, PRIMARY_MAKR_STYLE
from .commons import resolve_date_from_path


def processs_seq_exp(data_path: path.Path, plot_dir: path.Path):
    print(f'process_seq_exp: {data_path}, {plot_dir}', end=' ')
    data_file_date = resolve_date_from_path(data_path)
    print(f'resolved date: {data_file_date}')

    data_df = (
        pl.scan_csv(data_path, has_header=True)
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
        title="Czas wykonania alg. sekwencyjnego a rozmiar kubełków, dane: 256MB ~ 33e6 elem.",
        xlabel="Rozmiar kubełka",
        ylabel="Czas wykonania [s]")
    plot.grid()
    plot.legend()
    fig.tight_layout()
    fig.savefig(plot_dir.joinpath(f'seq-256-{data_file_date}.png'))
    plt.close(fig)

    print(data_df.sort('total_mean').head(10))

