# Makefile for CHIP-8 Emulator

CXX = g++
CXXFLAGS = -Wall -std=c++17
LDFLAGS = -lraylib
TARGET = build/emulator
SOURCES = emulator.cpp chip8.cpp
OBJECTS = $(patsubst %.cpp, build/%.o, $(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

build/%.o: %.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean