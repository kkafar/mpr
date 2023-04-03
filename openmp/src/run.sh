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

n_series=10
arr_size_base=100
arr_size_step=100
n_steps=1000

should_compile=1
should_run=1
should_process_data=1
should_archive=1


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

if [[ "${should_compile}" -eq 1 ]]
then
  gcc -fopenmp -std=c99 -O2 -o main main.c
fi

if [[ "${should_run}" -eq 1 ]]
then
  for (( sid = 1 ; sid <= ${n_series} ; sid++ ))
  do
    for (( stepid = 1 ; stepid <= ${n_steps} ; stepid++ ))
    do
      arrsize=$(( ${arr_size_base} + ${arr_size_step} * ${stepid} ))
      ./main ${arrsize} | tee "data/raw/series_${sid}_size_${arrsize}.csv"
    done
  done
fi




