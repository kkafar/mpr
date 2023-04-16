#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 1
#SBATCH --time=12:00:00
#SBATCH --partition=plgrid
#SBATCH --account=plgmpr23-cpu

function print_help () {
  echo """
  Usage: run [options]

  Options:
    -h -- print this help message and exit
    -a/A -- set all run flags to 1/0
    -c/C -- whether to compile
    -s N_SERIES -- run experiment N_SERIES times
    -r/R -- whether to run the experiment
    -d/D -- whether to process data or not
    -z/Z -- whether to archive data or not
    -n/N -- small/big array sizes
    -g/G -- whether to compile in debug mode; debug mode disables processing & archiving
    -t EXP_TYPE -- experiment type to run (async / sync)
    -p MIN_THREADS -- lower bound for number of threads
    -P MAX_THREADS -- upper bound for number of threads
    -i THREAD_STEP -- step for thread count
    -b MIN_BUCKET_SIZE -- lower bound for bucket size
    -B MAX_BUCKET_SIZE -- upper bound for bucket size
    -I BUCKET_SIZE_STEP -- step for bucket size
    -k -- whether to perform dry-run
  """
}

# $1 -- binary name
function assert_bin_exists () {
  if ! command -v "$1" &> /dev/null
  then
    echo "Looks like $1 binary is missing. Aborting."
    exit 1
  fi
}

rootdir="$(pwd)"

n_series=10
min_threads=1
max_threads=4
thread_step=1
min_bucket_size=1
max_bucket_size=20
bucket_size_step=1

should_compile=1
should_run=1
should_process_data=1
should_archive=1
debug_mode=0
exp_type="async"
is_dry_run=0

# number of doubles in...
half_GB=67108864
one_GB=134217728
two_GB=268435456
three_GB=536870912
four_GB=1073741824

arr_sizes=( ${half_GB} )

OPTIND=1
optstr="haAcCs:rRdDzZnNgGt:p:P:i:b:B:I:k"

while getopts "${optstr}" opt
do
  case "${opt}" in
    h)
      print_help
      exit 0
      ;;
    a)
      should_compile=1
      should_run=1
      should_process_data=1
      should_archive=1
      ;;
    A)
      should_compile=0
      should_run=0
      should_process_data=0
      should_archive=0
      ;;
    c)
      should_compile=1
      ;;
    C)
      should_compile=0
      ;;
    s)
      n_series="${OPTARG}"
      ;;
    r)
      should_run=1
      ;;
    R)
      should_run=0
      ;;
    d)
      should_process_data=1
      ;;
    D)
      should_process_data=0
      ;;
    z)
      should_archive=1
      ;;
    Z)
      should_archive=0
      ;;
    n)
      arr_sizes=( 128 )
      ;;
    N)
      ;;
    g)
      debug_mode=1
      should_process_data=0
      should_archive=0
      ;;
    G)
      debug_mode=0
      ;;
    t)
      exp_type="${OPTARG}"
      ;;
    p)
      min_threads="${OPTARG}"
      ;;
    P)
      max_threads="${OPTARG}"
      ;;
    i)
      thread_step="${OPTARG}"
      ;;
    b)
      min_bucket_size="${OPTARG}"
      ;;
    B)
      max_bucket_size="${OPTARG}"
      ;;
    I)
      bucket_size_step="${OPTARG}"
      ;;
    k)
      is_dry_run=1
      ;;
  esac
done

shift $((OPTIND - 1))

if [[ $is_dry_run -eq 1 ]]
then
  echo "Performing dry run"
fi

echo "Removing data/ directory to avoid conflicts..."
rm -fr data/
mkdir -p data/{raw,processed}
mkdir -p data-arch

outfile_name="final-${exp_type}.csv"
outfile="data/processed/${outfile_name}"

if [[ "${should_compile}" -eq 1 ]]
then
  echo "Removing old build artifacts..."
  [[ $is_dry_run -eq 1 ]] ||  make clean
  if [[ "${debug_mode}" -eq 1 ]]
  then
    echo "Building debug configuration..."
    [[ $is_dry_run -eq 1 ]] || make debug
  else
    echo "Building release configuration..."
    [[ $is_dry_run -eq 1 ]] || make release
  fi
fi

if [[ "${should_run}" -eq 1 ]]
then
  for arr_size in "${arr_sizes[@]}"
  do
    for (( n_threads = ${min_threads} ; n_threads <= ${max_threads} ; n_threads = $(( ${n_threads} + ${thread_step} )) ))
    do
      for (( bucket_size = ${min_bucket_size} ; bucket_size <= ${max_bucket_size} ; bucket_size = $(( ${bucket_size} + ${bucket_size_step} )) ))
      do
        n_buckets=$(( ${arr_size} / ${bucket_size} ))
        echo "Running arr_size: ${arr_size}, n_threads: ${n_threads}, n_buckets: ${n_buckets}, n_series: ${n_series}, exp_type: ${exp_type}"
        [[ $is_dry_run -eq 1 ]] || (./sort ${arr_size} ${n_threads} ${n_buckets} ${n_series} "${exp_type}" 2>&1 | tee "data/raw/th_${n_threads}_size_${arr_size}_buckets_${n_buckets}.csv")
      done
    done
  done
fi

if [[ "${should_process_data}" -eq 1 ]]
then
  echo "Processing data..."
  [[ $is_dry_run -eq 1 ]] || (echo "sid,arrsize,bsize,nthreads,total,draw,scatter,sort,gather" > ${outfile})
  cd data/raw

  files=$(ls .)
  echo -e "Processing:\n${files}"
  [[ $is_dry_run -eq 1 ]] || (ls . | xargs -n 1 tail -n +2 >> "../processed/${outfile_name}")
fi

if [[ ${should_archive} -eq 1 ]]
then
  cd $rootdir
  echo "Archiving final data..."
  timestamp=$(date +%Y-%m-%d-%H-%M-%S)
  archivefile="data-arch/${outfile_name}-${timestamp}.csv"
  mkdir -p "data-arch"
  [[ $is_dry_run -eq 1 ]] || (cp "${outfile}" "${archivefile}")
fi

