CXX = g++

all: bms

bms: bms.o
	$(CXX) bms.o -o bms

bms.o: bms.cpp bms.h
	$(CXX) $(CXXFLAGS) -c bms.cpp -o bms.o

clean:
	rm bms bms.o