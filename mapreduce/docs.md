---
Title: Metody programowania równoległego - MPI
Author: Kacper Kafara
---

<script type="text/javascript" src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<script type="text/x-mathjax-config">
  MathJax.Hub.Config({ tex2jax: {inlineMath: [['$', '$']]}, messageStyle: "none" });
</script>

# Metody Programowania Równoległego - MapReduce

Autor: Kacper Kafara

# Spis treści

- [Metody Programowania Równoległego - MapReduce](#metody-programowania-równoległego---mapreduce)
- [Spis treści](#spis-treści)
- [Zadanie](#zadanie)
- [Środowisko i konfiguracja](#środowisko-i-konfiguracja)
- [Program](#program)
	- [Mapper](#mapper)
	- [Reducer](#reducer)
- [Wyniki pomiarów i wnioski](#wyniki-pomiarów-i-wnioski)
- [Kod źródłowy](#kod-źródłowy)
	- [Mapper](#mapper-1)
	- [Reducer](#reducer-1)
	- [Wykresy](#wykresy)
	- [Konfiguracja węzłów \& eksperymenty](#konfiguracja-węzłów--eksperymenty)
		- [Przygotowanie węzła](#przygotowanie-węzła)

# Zadanie

Celem zadania było porównanie wydajności zoptymalizowanej wersji programu sekwencyjnego w stosunku do zrównoleglonej (paradygmat `MapReduce`).

Testowany był algorytm zliczania słów w tekście.

# Środowisko i konfiguracja

Eksperyment został wykonany przy wykorzystaniu udostępnionych zasobów AWS.

Wersja sekwencyjna programu została uruchomiona na pojedynczej instancji serwisu `EC2` o typie `m4.large` (2 rdzenie) (próby uruchamiania większych węzłów nie powodziły się (AWS rzucał błędy)).

Obliczenia równoległe na `EMR` dla danych wielkości 1GB, 5GB i 10GB były wykonywane na 6-ściu różnych klastrach:

1. 4 x 4 rdzenie (m4.xlarge), 16 rdzeni razem,
2. 8 x 2 rdzenie (m4.large), 16 rdzeni razem,
3. 8 x 4 rdzenie (m4.xlarge), 32 rdzenie razem,
4. 16 x 2 rdzenie (m4.large), 32 rdzenie razem,
5. 12 x 4 rdzenie (m4.xlarge), 48 rdzeni razem,
6. 24 x 2 rdzenie (m4.large), 48 rdzeni razem

Każde obliczenie zostało powtórzone 3-krotnie. Prezentowane wyniki to średnia z pomiarów.

# Program

Kod mappera i reducera został napisany w języku `Rust`. Ze względu na zwięzłość implementacji zamieszam kod poniżej.

## Mapper

```rust
///! Mapper
use std::io::Write;

fn main() {
    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin().lines().for_each(|line| {
        line.unwrap_or("".to_owned()).split_whitespace().for_each(|word| {
            stdout.write_fmt(format_args!("{word},1\n"));
        });
    });
}
```

## Reducer

```rust
///! Reducer
use std::{collections::HashMap, io::Write};

fn main() {
    let mut wtc: HashMap<String, usize> = HashMap::new();

    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin()
        .lines()
        .filter_map(|line| line.ok())
        .map(|line| {
            let split = line.split_once(',').unwrap();
            (split.0.to_owned(), split.1.to_owned())
        })
        .map(|(word, str_count)| {
            (word, str_count.parse::<usize>().unwrap_or(0))
        })
        .for_each(|(word, count)| {
            if let Some(crt_count) = wtc.get_mut(&word) {
                *crt_count += count;
            } else {
                wtc.insert(word, count);
            }
        });

    wtc.iter().for_each(|(word, count)| { stdout.write_fmt(format_args!("{word},{count}\n")); });
}
```

Istotne jest, że odczyty ze standardowego wejścia procesu są automatycznie buferowane (czyli liczba odczytów z pliku jest znacząco zredukowana).
Wpisy na standardowe wyjście buferuję samodzielnie korzystając z `std::io::BufWriter`.

# Wyniki pomiarów i wnioski

| ![time-size](plots/time-size.png) |
|:--:|
| *Wykres 1: Zależność czasu wykonania od rozmiaru problemu dla różnych konfiguracji* |


Na wykresie 1 obserwujemy, że tylko 3 konfiguracje dają lepsze wyniki od algorytmu sekwencyjnego:

1. 8 x 4 rdzenie (m4.xlarge) (od 10GB)
2. 24 x 2 rdzenie (m4.large) (od 5GB)
3. 12 x 4 rdzenie (m4.large) (od 5GB)

Żadna inna konfiguracja jest nie jest opłacalna.

| ![time-core-1](plots/time-cores-psize-1.png) |
|:--:|
| *Wykres 2: Zależność czasu wykonania od liczby rdzeni (dane: 1GB)* |

| ![time-core-5](plots/time-cores-psize-5.png) |
|:--:|
| *Wykres 3: Zależność czasu wykonania od liczby rdzeni (dane: 5GB)* |

| ![time-core-10](plots/time-cores-psize-10.png) |
|:--:|
| *Wykres 4: Zależność czasu wykonania od liczby rdzeni (dane: 10GB)* |

Z wykresów 2, 3 i 4 odczytujemy, że w przypadku małych danych (1 GB) przetwarzanie takiego problemu przy użyciu paradygmatu
MapReduce nie ma sensu (wykres 2 -- czasy MR nawet nie zbliżają się do czasu przetwarzania sekwencyjnego). W przypadku danych rzędu 5 GB opłacalne (**czasowo**)
jest wykorzystanie >= 48 rdzeni, a w przypadku 10GB nawet >= 32 rdzeni. Są to też jedyne przypadki, w których możemy określić metrykę COST.

Poniżej zamieszczam jeszcze wykresy przyśpieszenia, bez dodatkowej analizy.

| ![sp-core-1](plots/sp-cores-psize-1.png) |
|:--:|
| *Wykres 5: Przyśpieszenie (dane: 1GB)* |

| ![sp-core-5](plots/sp-cores-psize-5.png) |
|:--:|
| *Wykres 6: Przyśpieszenie (dane: 5GB)* |

| ![p-core-10](plots/sp-cores-psize-10.png) |
|:--:|
| *Wykres 7: Przyśpieszenie (dane: 10GB)* |

Ostatecznie obserwujemy zachowanie zgodne z opisem w artykule ["Scalability! But at what COST?" (F. McSherry, M. Isard, D. G. Murray)](https://www.usenix.org/system/files/conference/hotos15/hotos15-paper-mcsherry.pdf) -- zrównoleglanie programu nie zawsze
skutkuje zyskami w czasie wykonania. Przeciwnie, aby implementować równoległe rozwiązanie danego problemu należy mieć do tego istotne przesłanki (np. osiągnięcie limitów możliwości i optymalizacji systemu sekwencyjnego). Dla stosunkowo małych danych jest to szczegónie nieopłacalne.
Taka sytuacja wynika z faktu dużych narzutów wprowadzanych przez systemy realizujące obliczenia równoległe, a ich teoretyczne dobre skalowanie wynika tylko z faktu nie porównywania się z sensowną (optymalną) implementacją przetwarzania sekwencyjnego.
Zwracam też uwagę, że MapReduce korzystał z tego samego algorytmu co wersja sekwencyjna -- gdyby wykorzystać mniej wydajny algorytm -- ta różnica byłaby jeszcze większa i być może metryka COST nie byłaby nigdzie określona.


# Kod źródłowy

Do znalezienia w załączniku oraz poniżej.

## Mapper

```rust
///! Mapper
use std::io::Write;

fn main() {
    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin().lines().for_each(|line| {
        line.unwrap_or("".to_owned()).split_whitespace().for_each(|word| {
            stdout.write_fmt(format_args!("{word},1\n"));
        });
    });
}
```

## Reducer

```rust
///! Reducer
use std::{collections::HashMap, io::Write};

fn main() {
    let mut wtc: HashMap<String, usize> = HashMap::new();

    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin()
        .lines()
        .filter_map(|line| line.ok())
        .map(|line| {
            let split = line.split_once(',').unwrap();
            (split.0.to_owned(), split.1.to_owned())
        })
        .map(|(word, str_count)| {
            (word, str_count.parse::<usize>().unwrap_or(0))
        })
        .for_each(|(word, count)| {
            if let Some(crt_count) = wtc.get_mut(&word) {
                *crt_count += count;
            } else {
                wtc.insert(word, count);
            }
        });

    wtc.iter().for_each(|(word, count)| { stdout.write_fmt(format_args!("{word},{count}\n")); });
}
```

## Wykresy

```python
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
```

## Konfiguracja węzłów & eksperymenty

### Przygotowanie węzła

`compile-rust.sh`:

```bash
#!/usr/bin/env bash

root_dir=$(pwd)
cd rust/mapper
cargo build --release
cargo install --root ${root_dir} --path .

cd ../reducer
cargo build --release
cargo install --root ${root_dir} --path .
cd ${root_dir}

mv bin/* .
```

`prepare-node.sh`:

```bash
#!/usr/bin/env bash

# Rust installation
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env

# Package manager update
sudo yum update -y

# Git installation
sudo yum groupinstall "Development Tools"

# Getting repo
git clone https://github.com/kkafar/mpr.git mpr
cd mpr
git switch @kkafar/mapreduce
cd mapreduce
./compile-rust.sh
```

`run-mapreduce.sh`:

```bash
#!/usr/bin/env bash

sizes=("1" "5" "10")

mode="unknown"
mapper="./rust/mapper/target/release/mapper"
reducer="./rust/reducer/target/release/reducer"

bucket_url="s3://bucket-mpr"
input_file="${bucket_url}/data-1GB.txt"
output_dir="results"

if [[ $# -gt 0 ]]
then
  mode="$1"
fi

if [[ $# -gt 1 ]]
then
  mapper="$2"
fi

if [[ $# -gt 2 ]]
then
  reducer="$3"
fi

if [[ $# -gt 3 ]]
then
  input_file="$4"
fi

# Workaround to avoid using shell buitin `time` cmd
time_cmd="$(which time)"
${time_cmd} --format "%e" hadoop jar /usr/lib/hadoop/hadoop-streaming.jar \
  -files "${mapper}","${reducer}" \
  -mapper "${mapper}" \
  -reducer "${reducer}" \
  -input "${input_file}" \
  -output "${output_dir}"

hdfs dfs -get ${output_dir}
hdfs dfs -rm -r ${output_dir}

```

`run-seq.sh`:

```bash
#!/usr/bin/env bash

# This script should be executed only from the directory it is placed in the repository.

# $1 -- path to data directory
# $2 -- path to mapper
# $3 -- path to reducer

data_dir="data-mock"
data_files=("data-1GB.txt" "data-5GB.txt" "data-10GB.txt")
rust_mapper="rust/mapper/target/release/mapper"
rust_reducer="rust/reducer/target/release/reducer"
mode="unknown"
sizes=("1" "5" "10")

if [[ $# -gt 0 ]]
then
  data_dir="$1"
fi

if [[ $# -gt 1 ]]
then
  rust_mapper="$2"
fi

if [[ $# -gt 2 ]]
then
  rust_reducer="$3"
fi

if [[ $# -gt 3 ]]
then
  mode="$4"
fi

# Workaround to avoid using shell buitin `time` cmd
time_cmd="$(which time)"

mkdir -p results/tmp
result_file="results/result-$(date +%y-%m-%d-%H-%M-%S).txt"
tmp_file="results/tmp/tmp-${result-file}"

echo "time,mode,size" > "${result_file}"

for i in $(seq 0 1 "$(( ${#data_files[@]} - 1))")
do
  file="${data_files[${i}]}"
  size="${sizes[${i}]}"
  # echo "Running for file ${file}, size ${size}"
  "${time_cmd}" --format="%e" --output ${tmp_file} cat ${data_dir}/${file} | ${rust_mapper} | ${rust_reducer} &> /dev/null
  cat ${tmp_file} | awk -v mode="${mode}" -v size="${size}" -F ',' '{ print $0 "," mode "," size }' >> ${result_file}
done

rm -fr "results/tmp/"
```
