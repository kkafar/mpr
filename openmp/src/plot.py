import pathlib as path
import matplotlib.pyplot as plt
import sys
from plots.par import process_par_exp
from plots.seq import processs_seq_exp
from plots.prng import process_prng_exp


def deduce_expname_from_filename(filename: str) -> str:
    expname = filename.split('-')[1]
    if expname == "async":
        return "par"
    elif expname == "sync":
        return "seq"
    elif expname == "prng":
        return "prng"
    else:
        return "unknown"


data_dir = path.Path('data-arch', 'ares')
data_files = {
    "par": data_dir.joinpath('final-async-2023-04-17-15-17-02.csv'),
    "seq": data_dir.joinpath('final-sync-2023-04-19-15-00-00.csv')
}

plot_dir = path.Path('plots', 'sort')
if not plot_dir.is_dir():
    plot_dir.mkdir(parents=True, exist_ok=True)
assert plot_dir.is_dir(), "Plot directory exists"

if len(sys.argv) == 1:
    assert data_files["seq"].is_file(), "Seq data file exists"
    assert data_files["par"].is_file(), "Par data file exists"

    processs_seq_exp(data_files["seq"], plot_dir)
    process_par_exp(data_files["par"], plot_dir)

if len(sys.argv) == 2:
    # path to datafile is expected, expname is resolved based on filename
    # or if the arg == "all" -- all datafiles are plotted

    if sys.argv[1] == "all":
        for data_file in data_dir.glob('*.csv'):
            expname = deduce_expname_from_filename(data_file.name)
            assert expname == "par" or expname == "seq" or expname == "prng", "Correct exp name"

            if expname == "par":
                process_par_exp(data_file, plot_dir)
            else:
                processs_seq_exp(data_file, plot_dir)
    else:
        data_file = path.Path(sys.argv[1])
        assert data_file.is_file(), "Data file exists"

        expname = deduce_expname_from_filename(data_file.name)
        assert expname == "par" or expname == "seq" or expname == "prng", "Correct exp name"

        if expname == "par":
            process_par_exp(data_file, plot_dir)
        else:
            processs_seq_exp(data_file, plot_dir)

if len(sys.argv) == 3:  # exptype, datafile
    expname = sys.argv[1]
    assert expname == "par" or expname == "seq" or expname == "prng", "Correct exp name"

    data_file = path.Path(sys.argv[2])
    assert data_file.is_file(), "Data file exists"

    if expname == "par":
        process_par_exp(data_file, plot_dir)
    elif expname == "seq":
        processs_seq_exp(data_file, plot_dir)
    else:
        process_prng_exp(data_file, plot_dir)

plt.show()

