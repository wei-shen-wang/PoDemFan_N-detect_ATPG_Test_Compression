#

CFLAGS = -std=c++11 -g  

FILES = atpg.o main.o input.o level.o sim.o podem.o init_flist.o faultsim.o tdfsim.o display.o

all : $(FILES)
	g++ $(CFLAGS) $(FILES) -lm -o atpg
atpg.o : atpg.h atpg.cpp
	g++ $(CFLAGS) -c atpg.cpp
main.o : atpg.h main.cpp
	g++ $(CFLAGS) -c main.cpp
input.o : atpg.h input.cpp
	g++ $(CFLAGS) -c input.cpp
level.o : atpg.h level.cpp
	g++ $(CFLAGS) -c level.cpp
sim.o : atpg.h logic_tbl.h sim.cpp
	g++ $(CFLAGS) -c sim.cpp
podem.o : atpg.h podem.cpp
	g++ $(CFLAGS) -c podem.cpp
init_flist.o : atpg.h init_flist.cpp
	g++ $(CFLAGS) -c init_flist.cpp
faultsim.o : atpg.h faultsim.cpp
	g++ $(CFLAGS) -c faultsim.cpp
tdfsim.o : atpg.h tdfsim.cpp
	g++ $(CFLAGS) -c tdfsim.cpp
display.o : atpg.h display.cpp
	g++ $(CFLAGS) -c display.cpp
clean:
	rm *.o atpg	
