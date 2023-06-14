# ATPG_TDF_ndetect_compression
2023 VLSI Testing Final Project

In all three directories FAN_PODEM_PARALLEL, FANV2_PODEMV1, PODEM
- type make to compile the code into executables
- type make clean to clean the executables

Command :
```sh
./atpg -tdfatpg -ndet n -compression *.ckt > *.pat
```
## PODEM

Implement and modify the PODEM algorothim to generate patterns for transition delay fault.
This part is done by [Hsin-Tzu Zhang](https://github.com/EE08053).

## PODEM V1 + FAN V2

Implement FAN to generate V2 and PODEM to generate V1 for transition delay faults. This part is done by [Wei-Shen Wang](https://github.com/wei-shen-wang).

Original FAN algorithm source code : [https://github.com/NTU-LaDS-II/FAN_ATPG](https://github.com/NTU-LaDS-II/FAN_ATPG).

The circuit ckt-to-verilog conversion for FAN_ATPG is done by Hsin-Tzu Zhang and Yu-Hung Pan.

The modified command line is done by Yu-Hung Pan.

<!---
This part is deprecated

### Additional command (debug use)

To run the fault simulation for transition delay fault in FAN package, you can type the command :
```sh
./atpg -ndet n -tdfsim *.pat *.ckt
```

To generate the pattern with specific pattern format for FAN, you can additionally type **-rpf** flag. This will additionally generate the pattern file **./tdf_test.pat**.
--->

## Parallel Processing

Run two ATPG in parallel and choose better generated pattern. This part is done by [Yu-Hung Pan](https://github.com/PAN-YU-HUNG).

## Statistics for reference

Ndet : **8**, Compression : **ON**

PODEM :
| Circuit | FC(%) | TL | RT(s) |
| :----: | :----: | :----: | :----: |
|c432|11.62% |170 |0.8|
|c499|94.69%|582|16.6|
|c880|50.38%|299|8.7|
|c1355|38.41%|463|5.0|
|c2670|94.06%|874|60.3|
|c3540|23.26%|486|76.8|
|c6288|97.63%|387|75.2|
|c7522|98.31%|1199|433.5|
|Average|63.55%|557.5 |84.6|

PODEM V1 + FAN V2 :
| Circuit | FC(%) | TL | RT(s) |
| :----: | :----: | :----: | :----: |
|c432|11.62%|170|0.51|
|c499|95.56%|394|7.28|
|c880|50.33%|368|22.28|
|c1355|38.41%|469|7.45|
|c2670|94.13%|883|428|
|c3540|23.26%|495|190.4|
|c6288|97.66%|375|322.7|
|c7522|98.21%|1945|1176|
|Average|63.65%|637.4|269.3|