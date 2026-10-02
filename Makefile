CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
SRC_LIB  := src/lexique.cpp src/lexique_ligne.cpp src/utilitaire.cpp
INC      := -Isrc

all: tp1 tests_tp1

tp1: $(SRC_LIB) src/main.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) $(INC) $(SRC_LIB) src/main.cpp -o $@

tests_tp1: $(SRC_LIB) tests/tests.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) $(INC) $(SRC_LIB) tests/tests.cpp -o $@

run: tp1
	./tp1 data

test: tests_tp1
	./tests_tp1 tests

clean:
	rm -f tp1 tests_tp1 lexique_*.txt

.PHONY: all run test clean
