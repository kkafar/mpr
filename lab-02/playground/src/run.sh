echo "Running run.sh script"

# possible params
progname="main"
machinefilename="nodes"
point_counts=(100000 10000000 100000000)
proc_counts=(1 2 4 8 12)

# execution context
is_ares=1
execution_context="vCluster"

# Chceck whether necessary binaries are installed
if ! command -v mpiexec &> /dev/null then
  echo "Looks like mpiexec binary is missing... Aborting"
  exit 1
fi

# Detect on what machine we're running.
# Currently it is up to user to specify env var IS_ARES=1
# in case the script is run on Ares
if [[ -z "${IS_ARES}" ]]; then
  echo "Execution context: Ares"
  is_ares=0
  execution_context="Ares"
else
  echo "Execution context: vCluster"
  execution_context="vCluster"
fi

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
      # mpiexec -machinefile "./${machinefilename}" -np ${cur_proc_count} "./${progname}" "${cur_point_count}" > "${output_raw}/proc_${cur_proc_count}_point_${cur_point_count}.csv"
    fi
  done
done

exit 0

