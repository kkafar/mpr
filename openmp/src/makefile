.PHONY: all clean release
.DEFAULT: all

all: release

release: sort.cpp
	g++ -fopenmp -std=c++11 -O2 -o sort sort.cpp

debug: sort.cpp
	g++ -fopenmp -ggdb -std=c++11 -DDEBUG -o sort sort.cpp

clean:
	rm -fr sort

