#!/bin/bash
# ------------------------------------------------------------------
# run_all.sh  -  build the simulator, run every configuration and
#                export the results to CSV for the Python analysis.
#
# Run it from the OMNeT++ shell (mingwenv.cmd), inside this folder:
#     ./run_all.sh
# ------------------------------------------------------------------
set -e

# 1) build
opp_makemake -f --deep -o VehicleSim
make MODE=release -j8

# 2) run every configuration, 8 runs in parallel
CONFIGS="Validation_DD1 Validation_MD1 Exponential Lognormal LognormalCV IdealHandover TimeSeries"
for CFG in $CONFIGS; do
    echo "=== running $CFG ==="
    opp_runall -j8 ./out/clang-release/VehicleSim -u Cmdenv -c $CFG -s
done

# 3) export the results of each configuration to one CSV file (read by analyze_results.py)
#    we pass the FOLDER (results/$CFG), not the file list: 1440 file names are too long a command
mkdir -p ../analysis/data
for CFG in $CONFIGS; do
    opp_scavetool export -F CSV-R -o ../analysis/data/$CFG.csv results/$CFG
done

echo "Done. CSV files are in ../analysis/data/"
