import polars as pl
import matplotlib.pyplot as plt
import pathlib as path

COL_NAMES = ['sid', 'nelems', 'ebsize', 'nbuckets', 'bid', 'bsize']


def process_prng_exp(data_file: path.Path, plot_dir: path.Path):
    print(f'process_prng_exp: {data_file}, {plot_dir}')

    data_df = (
        pl.scan_csv(data_file, has_header=True)
        .groupby(['nelems', 'ebsize', 'nbuckets', 'bid'])
        .agg([
            pl.col('bsize').mean().alias('bsize_mean'),
            pl.col('bsize').std().alias('bsize_std')
        ])
        .sort('bid')
        .collect()
    )

    n_elems = data_df.get_column('nelems')[0]
    eb_size = data_df.get_column('ebsize')[0]
    n_buckets = data_df.get_column('nbuckets')[0]

    fig, plot = plt.subplots(nrows=1, ncols=1)

    x_data = data_df.get_column('bid')
    y_data = data_df['bsize_mean']
    y_error = data_df['bsize_std']

    plot.bar(x_data, y_data, yerr=y_error, label="Liczba elem. w kubełku")
    plot.plot(x_data, [eb_size for _ in range(len(x_data))], label="Wartość oczekiwana",
              color="red", linestyle=":")

    plot.set(
        title=f"Rozkład generatora erand48, {n_elems} elementów",
        xlabel="Nr przedziału (kubełek)",
        ylabel="Liczba elementów w kubełku")
    plot.grid()
    plot.legend()
    fig.tight_layout()
    fig.savefig(plot_dir.joinpath(f'prng-distribution-{n_elems}-{n_buckets}.png'))
    # plt.close(fig)

    print(data_df)

