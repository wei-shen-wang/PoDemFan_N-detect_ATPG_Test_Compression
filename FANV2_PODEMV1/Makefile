#

CFLAGS = -std=c++11 -Ofast

FILES = main.o

all : $(FILES)
	make -f fan.mk
	g++ $(CFLAGS) $(FILES) -lm -o atpg

main.o : convert.cpp main.cpp
	g++ $(CFLAGS) -c main.cpp

clean :
	make -f fan.mk clean
	rm *.o atpg