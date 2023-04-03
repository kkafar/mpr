#!/bin/bash

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
arr_size_base=100
arr_size_step=100
n_steps=1000

should_compile=1
should_run=1
should_process_data=1
should_archive=1

# number of doubles in...
one_GB=134217728
two_GB=268435456
three_GB=536870912
four_GB=1073741824

arr_sizes=( ${one_GB} ${two_GB} ${three_GB} ${four_GB} )
# arr_sizes=( 32768 )

OPTIND=1
optstr="haAcCs:rRdDzZ"

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
  esac
done

shift $((OPTIND - 1))

mkdir -p data/{raw,processed}
mkdir -p data-arch

outfile="data/processed/final.csv"

if [[ "${should_compile}" -eq 1 ]]
then
  gcc -fopenmp -std=c99 -O2 -o main main.c
fi

if [[ "${should_run}" -eq 1 ]]
then
  for (( sid = 1 ; sid <= ${n_series} ; sid++ ))
  do
    for arrsize in "${arr_sizes[@]}"
    do
      for (( nthreads = 1 ; nthreads <= 4 ; nthreads++ ))
      do
        echo "Running sid: ${sid}, nthreads: ${nthreads}, arrsize: ${arrsize}"
        ./main ${arrsize} ${nthreads} | tee "data/raw/sid_${sid}_th_${nthreads}_size_${arrsize}.csv"
        # ./main ${arrsize} ${nthreads}
      done
    done
  done
fi

if [[ "${should_process_data}" -eq 1 ]]
then
  echo "Processing data..."
  echo "sid,type,threads,chunk,size,time" > ${outfile}
  for (( sid = 1 ; sid < ${n_series} ; sid++ ))
  do
    for arrsize in "${arr_sizes[@]}"
    do
      for (( nthreads = 1 ; nthreads <= 4 ; nthreads++ ))
      do
        echo "Processing sid: ${sid}, nthreads: ${nthreads}, arrsize: ${arrsize}"
        cat "data/raw/sid_${sid}_th_${nthreads}_size_${arrsize}.csv" | tail -n 1 | awk -v sid=${sid} -F ',' '/.+/ {print sid "," $0}' >> ${outfile}
      done
    done
  done
fi

if [[ ${should_archive} -eq 1 ]]
then
  echo "Archiving final data..."
  timestamp=$(date +%Y-%m-%d-%H-%M-%S)
  archivefile="data-arch/final-${timestamp}.csv"
  mkdir -p "data-arch"
  cp "${outfile}" "${archivefile}"
fi


