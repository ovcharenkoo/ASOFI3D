#!/usr/bin/env bash
# Regression test 05.
# Check the reciprocity.
# Precisely, we do the following:
# Run simulation 1 with source type "force in x-direction" and measure v_y
# in receiver;
# Run simulation 2 with source type "force in y-direction" and measure v_x
# in the receiver. Here the source and the receiver are swapped.
# Check that the seismograms from these simulations are the same.
# We consider isotropic fullspace (one-layer) medium.
. tests/functions.sh

readonly MODEL="src/model_elastic.c"
readonly TEST_PATH="tests/fixtures/test_05"
readonly TEST_ID="TEST_05"

setup

backup_default_model

# Copy test model.
cp "${TEST_PATH}/model_elastic.c"            src/
cp "${TEST_PATH}/asofi3D_force_in_x.json"    tmp/in_and_out/asofi3D.json
cp "${TEST_PATH}/source_force_in_x.dat"      tmp/sources/
cp "${TEST_PATH}/receiver_force_in_x.dat"    tmp/receiver/

compile_code

run_solver np=16 dir=tmp log=ASOFI3D.log

cp "${TEST_PATH}/asofi3D_force_in_y.json"    tmp/in_and_out/asofi3D.json
cp "${TEST_PATH}/source_force_in_y.dat"      tmp/sources/
cp "${TEST_PATH}/receiver_force_in_y.dat"    tmp/receiver/

run_solver np=16 dir=tmp log=ASOFI3D.log

# Convert seismograms in SEG-Y format to the Madagascar RSF format.
convert_segy_to_rsf tmp/su/force_in_x_vy.sgy
convert_segy_to_rsf tmp/su/force_in_y_vx.sgy

# Read the files.
tests/compare_datasets.py tmp/su/force_in_x_vy.rsf tmp/su/force_in_y_vx.rsf \
    --rtol=1e-15 --atol=1e-15
result=$?
if [ "$result" -ne "0" ]; then
    log "Traces differ"
fi

log "PASS"
