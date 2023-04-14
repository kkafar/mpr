.PHONY: all clean
.DEFAULT: all

all: sort

sort: sort.cpp
	g++ -fopenmp -o sort sort.cpp

debug: sort.cpp
	g++ -fopenmp -ggdb -DDEBUG -o sort sort.cpp

clean:
	rm -fr sort

