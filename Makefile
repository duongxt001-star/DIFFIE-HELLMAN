ROOT ?= .

CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -std=c++14 -O3 -Wall -Wextra -Wpedantic
CFLAGS ?= -O3 -Wall -Wextra -Wpedantic

BIN := $(ROOT)/bin
BUILD := $(BIN)/build

PRNG_OBJECTS := $(BUILD)/main_prng.o $(BUILD)/prng.o $(BUILD)/fips202.o $(BUILD)/u512.o

.PHONY: all folders run-prng clean

all: $(BIN)/prng

folders:
	mkdir -p $(BUILD) $(BIN)

$(BUILD)/main_prng.o: $(ROOT)/main_prng.cpp $(ROOT)/prng.h $(ROOT)/fips202.h $(ROOT)/u512.h | folders
	$(CXX) $(CXXFLAGS) -I$(ROOT) -c $< -o $@

$(BUILD)/prng.o: $(ROOT)/prng.cpp $(ROOT)/prng.h $(ROOT)/fips202.h | folders
	$(CXX) $(CXXFLAGS) -I$(ROOT) -c $< -o $@

$(BUILD)/u512.o: $(ROOT)/u512.cpp $(ROOT)/u512.h | folders
	$(CXX) $(CXXFLAGS) -I$(ROOT) -c $< -o $@

$(BUILD)/fips202.o: $(ROOT)/fips202.c $(ROOT)/fips202.h | folders
	$(CC) $(CFLAGS) -I$(ROOT) -c $< -o $@

$(BIN)/prng: $(PRNG_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

run-prng: $(BIN)/prng
	./$(BIN)/prng

clean:
	rm -rf $(BIN)
