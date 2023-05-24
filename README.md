# ATPG_TDF_ndetect_compression
2023 VLSI Testing Final Project

Command : ./atpg -tdfatpg -ndet n -compression *.ckt > *.pat

## Statistic for SAF ATPG
## (Compression on, fault collapsing on, MFO off)

c17 : Doesn't in the report , patterns : 8 -> 5

c432 : same

c499 : patterns : 66 -> 38, FC : 94.69% -> 96.99%

c880 : patterns : 65 -> 27

c1355 : patterns : 63 -> 51

c2670 : patterns : 135 -> 126, FC : 96.29% -> 96.23%

c3540 : patterns : 98 -> 50

c5315 : No comparison, doesn't in the report

c6288 : patterns : 42 -> 26

c7552 : patterns : 289 -> 142, FC : 98.40% -> 98.43%