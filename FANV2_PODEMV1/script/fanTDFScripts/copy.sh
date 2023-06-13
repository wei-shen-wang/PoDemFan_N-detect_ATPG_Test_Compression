#!/bin/bash

sed 's/c17/c432/g' atpg_c17.script > atpg_c432.script;
sed 's/c17/c432/g' fsim_c17.script > fsim_c432.script;
sed 's/c17/c499/g' atpg_c17.script > atpg_c499.script;
sed 's/c17/c499/g' fsim_c17.script > fsim_c499.script;
sed 's/c17/c880/g' atpg_c17.script > atpg_c880.script;
sed 's/c17/c880/g' fsim_c17.script > fsim_c880.script;
sed 's/c17/c1355/g' atpg_c17.script > atpg_c1355.script;
sed 's/c17/c1355/g' fsim_c17.script > fsim_c1355.script;
sed 's/c17/c2670/g' atpg_c17.script > atpg_c2670.script;
sed 's/c17/c2670/g' fsim_c17.script > fsim_c2670.script;
sed 's/c17/c3540/g' atpg_c17.script > atpg_c3540.script;
sed 's/c17/c3540/g' fsim_c17.script > fsim_c3540.script;
sed 's/c17/c5315/g' atpg_c17.script > atpg_c5315.script;
sed 's/c17/c5315/g' fsim_c17.script > fsim_c5315.script;
sed 's/c17/c6288/g' atpg_c17.script > atpg_c6288.script;
sed 's/c17/c6288/g' fsim_c17.script > fsim_c6288.script;
sed 's/c17/c7552/g' atpg_c17.script > atpg_c7552.script;
sed 's/c17/c7552/g' fsim_c17.script > fsim_c7552.script;
#declare -a ckt_name=("c432" "c499" "c880" "c1355" "c2670" "c3540" "c5315" "c6288" "c7552")

#for str in  ${ckt_name[@]}; do
  #cp atpg_c17.script atpg_temp.script;
  #sed 's/c17/$str/g' atpg_c17.script > atpg_${str}.script;
  #cp fsim_c17.script fsim_temp.script;
  #sed 's/c17/$str/g' fsim_c17.script > fsim_${str}.script;
#done
#rm atpg_temp.script
#rm fsim_temp.script
echo "copy done"
