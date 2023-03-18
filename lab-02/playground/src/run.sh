#! /usr/bin/bash

echo "Running run.sh script"

print_help ()
{
  echo -e "Available params:\n\t-h -- show this help\n\t-b BINARY -- specify program to run (defaults to 'main')\n\t-c/-C -- whether to compile (uses make)\n\t-d/-D -- whether to process data\n\t-r/-R -- whether to run\n\t-a/-A -- run all stages / none; specify it always first\n\t-m MACHINEFILE"
}

# possible params
progname="main"
machinefilename="nodes"
point_counts=(100000 10000000 100000000)
proc_counts=(1 2 4 8 12)

should_process_data=1 # 1 means 'yes'
should_compile=1
should_run=1

# execution context
is_ares=1
execution_context="vCluster"

# https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
OPTIND=1
opt_str="hab:dDm:cCrR"

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
  esac
done

shift $((OPTIND-1))

if [[ ${should_compile} -eq 1 ]]
then
  if ! command -v make &> /dev/null
  then
    echo "Looks like make binary is missing... Aborting"
    exit 1
  fi
  make
fi

if [[ ${should_run} -eq 1 ]]
then
  if ! command -v mpiexec &> /dev/null
  then
    echo "Looks like mpiexec binary is missing... Aborting"
    exit 1
  fi

  if ! command -v tee &> /dev/null
  then
    echo "Looks like tee binary is missing... Aborting"
    exit 1
  fi

  # Detect on what machine we're running.
  # Currently it is up to user to specify env var IS_ARES=1
  # in case the script is run on Ares
  if [[ ! -z "${IS_ARES}" ]]
  then
    echo "Execution context: Ares"
    is_ares=0
    execution_context="Ares"
  else
    echo "Execution context: vCluster"
    execution_context="vCluster"
  fi

  if [[ ! -z "${MACHINEFILE}" ]]
  then
    machinefilename="${MACHINEFILE}"
  fi

  echo "Running with machinefile: ${machinefilename}"

  # Ensure that output output directory exists
  mkdir -p data/{raw,processed}

  output_raw="data/raw"
  output_processed="data/processed"

  for cur_proc_count in "${proc_counts[@]}"
  do
    for cur_point_count in "${point_counts[@]}"
    do
      if [[ ${is_ares} -eq 0 ]]; then
        echo "[${execution_context}] Execution commands for Ares are unimplemented; Aborting"
        exit 1
        # mpiexec -np ${cur_proc_count} "./${progname}" "${cur_point_count}" > "${output_raw}/proc_${cur_proc_count}_point_${cur_point_count}.csv"
      else
        echo "[${execution_context}] Point count: ${cur_point_count}, process count: ${cur_proc_count}"
        mpiexec -machinefile "./${machinefilename}" -np ${cur_proc_count} "./${progname}" "${cur_point_count}" | tee "${output_raw}/proc_${cur_proc_count}_point_${cur_point_count}.csv"
      fi
    done
  done
fi

if [[ ${should_process_data} -eq 1 ]]
then
  echo "Processing raw data..."
  if ! command -v xargs &> /dev/null
  then
    echo "Looks like xargs binary is missing... Aborting"
    exit 1
  fi

  raw_data="$(ls ${ouput_raw}/"
  echo "Detected files: ${raw_data}"

  finaldatafile="${output_processed}/final.csv"
  echo "proc_count,total_point_count,point_count,avg_pi,time" > "${finaldatafile}"

  ls "${output_raw}" | xargs -n 1 tail -n 1 >> "${finaldatafile}"
fi

exit 0

