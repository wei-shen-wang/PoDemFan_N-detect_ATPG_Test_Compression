#!/bin/bash
rm rpt/*
rm rpt_test/*
make clean
make &> make_cml.txt
#./bin/opt/fan -f script/fanScripts/atpg_s38584.script
#./bin/opt/fan -f script/fanScripts/atpg_c1355.script
bash script/run.sh script/fanGoldScripts
bash script/run.sh script/fanTestScripts
#bash script/autoRunScripts/runFAN.sh script/fanScripts