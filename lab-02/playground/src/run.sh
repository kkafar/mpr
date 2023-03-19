#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=01:00:00
#SBATCH --partition=plgrid
#SBATCH --account=plgmpr23-cpu

## vcluster
##! /usr/bin/bash

rootdir="$(pwd)"
echo "Running run.sh script from dir: ${rootdir}"

print_help ()
{
  echo -e "Available params:\n\t-h -- show this help\n\t-b BINARY -- specify program to run (defaults to 'main')\n\t-c/-C -- whether to compile (uses make)\n\t-d/-D -- whether to process data\n\t-r/-R -- whether to run\n\t-a/-A -- run all stages / none; specify it always first\n\t-m MACHINEFILE\n\t-x/-X -- ares / not ares execution context\n\t-t -- run with test params\n\t-s/-S -- scale weak / strong\n\t-z -- archive final data"
}

# $1 -- binary name
assert_binary_exists ()
{
  if ! command -v "$1" &> /dev/null
  then
    echo "Look like $1 binary is missing. Aborting"
    exit 1
  fi
}

run_vc_strong ()
{
  # Constant problem size -- splitted over various numbers of processes
  local problem_size=${vc_strong_point_count}
  local exp_type="strong"
  
  for (( series_id = 1 ; series_id <= ${vc_repeats} ; series_id++ ))
  do
    for n_processes in ${proc_count}
    do
      echo "[${execution_context}] Point count: ${problem_size}, process count: ${n_processes}"
      mpiexec -machinefile "./${machinefilename}" -np ${n_processes} "./${progname}" "${problem_size}" | tee "${outdir_raw}/type_${exp_type}_series_${series_id}_points_${problem_size}_procs_${n_processes}.csv"
    done
  done
}

run_vc_weak ()
{
  local base_problem_size=${vc_weak_point_count_base}
  local exp_type="weak"
  local problem_size=${base_problem_size}

  for (( series_id = 1 ; series_id <= ${vc_repeats} ; series_id++ ))
  do
    for n_processes in ${proc_count}
    do
      problem_size=$(( ${n_processes} * ${base_problem_size} ))
      echo "[${execution_context}] Point count: ${problem_size}, process count: ${n_processes}"
      mpiexec -machinefile "./${machinefilename}" -np ${n_processes} "./${progname}" "${problem_size}" | tee "${outdir_raw}/type_${exp_type}_series_${series_id}_points_${problem_size}_procs_${n_processes}.csv"
    done
  done
}

# run_ares_strong ()
# {
# }
#
# run_ares_weak ()
# {
# }

# possible params
progname="main"
machinefilename="allnodes"

# Shared configuration
proc_count=$(seq 1 1 12)

# vcluster configurations
vc_repeats=1

# weak scaling configuration
vc_weak_point_count_base=100000000 # 1e8

# strong scaling configuration
vc_strong_point_count=100000000 # 1e8

# Ares configurations
ares_repeats=1
ares_point_counts=( 1000 10000000 100000000000 ) # 1e3, 1e7, 1e11


# Actions to execute
should_process_data=1 # 1 means 'yes'
should_compile=1
should_run=1
should_archive=0
is_test=0


# Execution context
username="$(whoami)"

if [[ "${username}" != "kafara" ]]
then
  is_ares=1
  execution_context="ares"
else
  is_ares=0
  execution_context="vcluster"
fi


# https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
OPTIND=1
opt_str="haAb:dDm:cCrRxXtsSz"

while getopts "${opt_str}" opt
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
      ;;
    A)
      should_compile=0
      should_run=0
      should_process_data=0
      ;;
    b) progname="${OPTARG}"
      ;;
    d) should_process_data=1
      ;;
    D) should_process_data=0
      ;;
    m) machinefilename="${OPTARG}"
      ;;
    c) should_compile=1
      ;;
    C) should_compile=0
      ;;
    r) should_run=1
      ;;
    R) should_run=0
      ;;
    x) is_ares=1
      ;;
    X) is_ares=0
      ;;
    t) 
      is_test=1
      ;;
    z)
      should_archive=1
      ;;
  esac
done

shift $((OPTIND-1))

if [[ ${is_ares} -eq 1 ]]
then
  echo "Adding plgrid/tools/openmpi module"
  module add .plgrid plgrid/tools/openmpi
fi

if [[ ${should_compile} -eq 1 ]]
then
  echo "Compiling..."
  assert_binary_exists "make"
  make
fi

# Make sure directories are available
echo "Creating data directories"
mkdir -p data/{test,full}/{ares,vcluster}/{raw,processed}

if [[ ${is_test} -eq 1 ]]
then
  modedir="test"
else
  modedir="full"
fi

outdir="data/${modedir}/${execution_context}"
outdir_raw="${outdir}/raw"
outdir_processed="${outdir}/processed"

echo "Resolved outdir: ${outdir}"

if [[ ${should_run} -eq 1 ]]
then
  echo "Running..."
  assert_binary_exists "mpiexec"
  assert_binary_exists "tee"

  if [[ ${is_ares} -eq 0 && ! -z "${MACHINEFILE}" ]]
  then
    machinefilename="${MACHINEFILE}"
    echo "Running with machinefile: ${machinefilename}"
  fi

  # Main loop dispatch
  if [[ ${is_ares} -eq 1 ]]
  then
    echo "Running loop for Ares has not been implemented yet"
    exit 1
  else
    run_vc_strong
    run_vc_weak
  fi

  # for cur_proc_count in "${proc_counts[@]}"
  # do
  #   for cur_point_count in "${point_counts[@]}"
  #   do
  #     if [[ ${is_ares} -eq 1 ]]; then
  #       # echo "[${execution_context}] Execution commands for Ares are unimplemented; Aborting"
  #       # exit 1
  #       echo "[${execution_context}] Point count: ${cur_point_count}, process count: ${cur_proc_count}"
  #       mpiexec -np ${cur_proc_count} "./${progname}" "${cur_point_count}" | tee "${output_raw}/proc_${cur_proc_count}_point_${cur_point_count}.csv"
  #     else
  #       echo "[${execution_context}] Point count: ${cur_point_count}, process count: ${cur_proc_count}"
  #       mpiexec -machinefile "./${machinefilename}" -np ${cur_proc_count} "./${progname}" "${cur_point_count}" | tee "${output_raw}/proc_${cur_proc_count}_point_${cur_point_count}.csv"
  #     fi
  #   done
  # done
fi
#
if [[ ${should_process_data} -eq 1 ]]
then
  echo "Processing raw data..."
  assert_binary_exists "xargs"
  assert_binary_exists "awk"

  cd "${outdir_raw}"

  outfile=""
  for exptype in "strong" "weak"
  do
    echo "Processing for experiment type: ${exptype}"
    for (( series_id = 1 ; series_id <= ${vc_repeats} ; series_id++ ))
    do
      # outfile="final_type_${exptype}_series_${series_id}.csv"
      outfile="final.csv"
      echo "type,series,proc_count,total_point_count,point_count,avg_pi,time" > "../processed/${outfile}"
      ls . | grep "^type_${exptype}_series_${series_id}" | xargs -n 1 tail -n 1 | awk -v type="${exptype}" -v sid="${series_id}" -F ',' '/.+/ {print type "," sid "," $0}' >> "../processed/${outfile}"
    done
  done

  # cd "${outdir_processed}"
  #
  # outfile="final.csv"
  # for exptype in "strong" "weak"
  # do
  #
  # done

  cd "${rootdir}"

  # echo "proc_count,total_point_count,point_count,avg_pi,time" > "../processed/final.csv"
  # ls . | grep "^type_weak"
  # ls . | xargs -n 1 tail -n 1 >> "../processed/final.csv"
  # cd ../..
fi


if [[ ${should_archive} -eq 1 ]]
then
  cd "${rootdir}"
  echo "Archiving final data..."
  timestamp=$(date +%Y-%m-%d-%H:%M:%S)
  archivedir="data-archive/${execution_context}/${timestamp}"
  mkdir -p "${archivedir}"
  cp "${outdir_processed}/*.csv" "${archivedir}/"
fi

exit 0

