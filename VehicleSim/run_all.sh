#!/bin/bash
# ------------------------------------------------------------------
# run_all.sh  -  build the simulator, run every configuration and
#                export the results to CSV for the Python analysis.
#
# Run it from the OMNeT++ shell (mingwenv.cmd), inside this folder:
#     ./run_all.sh
# (in the IDE: build the project, then run the configs of simulations/omnetpp.ini)
# ------------------------------------------------------------------
set -e

# 1) build (src/Makefile and the *_m.cc/_m.h files are generated here)
cd src
opp_makemake -f --deep -o 5G-Connected-Vehicle
make MODE=release -j8
cd ../simulations

# 2) run every configuration, 8 runs in parallel
TESTS="Verification_DD1 Verification_Kinematics Verification_Kinematics_Handover Verification_Degeneracy_Alpha0
       Verification_Degeneracy_N1 Verification_Consistency_Base Verification_Consistency_scaled Verification_Continuity
       Validation_MD1 Calibration_Warmup"
EXPERIMENTS="Exp_Exponential Exp_Lognormal Exp_Ideal_Exponential Exp_Ideal_Lognormal Exp_ECDF_Exponential Exp_ECDF_Lognormal"
for CFG in $TESTS $EXPERIMENTS; do
    echo "=== running $CFG ==="
    opp_runall -j8 ../src/5G-Connected-Vehicle -u Cmdenv -n .:../src -c $CFG -s
done

# 3) export the results of each configuration to one CSV file (read by the notebook)
#    we pass the FOLDER, not the file list: thousands of file names are too long a command
mkdir -p ../../Analysis/data
for CFG in $TESTS $EXPERIMENTS; do
    DIR=$(find results -type d -name $CFG)
    opp_scavetool export -F CSV-R -o ../../Analysis/data/$CFG.csv $DIR
done

echo "Done. CSV files are in Analysis/data/"
