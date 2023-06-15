#! /bin/bash
myArray=("432" "499" "880" "1355" "2670" "3540" "6288" "7552")
for circuit_no in "${myArray[@]}"
do
  time ./atpg -tdfatpg -ndet 8 -compression sample_circuits/c${circuit_no}.ckt > tdf_patterns/c${circuit_no}_8.pat
  bin/golden_atpg -ndet 8 -tdfsim tdf_patterns/c${circuit_no}_8.pat sample_circuits/c${circuit_no}.ckt > rpt/c${circuit_no}_8.rpt
done