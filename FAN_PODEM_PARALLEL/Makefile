#

CFLAGS = -std=c++11 -Ofast

FILES = main.o

all : $(FILES)
	cd FAN_ATPG; make ; cd ..
	cd PODEM/src; make ; cd ../..
	g++ $(CFLAGS) $(FILES) -lm -o atpg

main.o : convert.cpp main.cpp
	g++ $(CFLAGS) -c main.cpp

clean :
	cd FAN_ATPG; make clean ; cd ..
	cd PODEM/src; make clean ; cd ../..
	rm *.o atpg