#!/bin/bash

declare -a ckt_name=("c17" "c1355" "c432" "c5315" "c6288")

for str in  ${ckt_name[@]}; do
  #cp atpg_c17.script atpg_temp.script;
  sed 's/tsmc/mod_tsmc/g' atpg_${str}.script > atpg_${str}.script;
  sed 's/tsmc/mod_tsmc/g' fsim_${str}.script > fsim_${str}.script;
  #cp fsim_c17.script fsim_temp.script;
  #sed 's/c17/$str/g' fsim_c17.script > fsim_${str}.script;
done
#rm atpg_temp.script
#rm fsim_temp.script
echo "copy done"
