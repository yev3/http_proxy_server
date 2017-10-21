###############################################################################
# Project1, CPSC 5510 Networking, Seattle University
# Yevgeni Kamenski
# 
# Makefile - compiles the fingerclient and fingerserver programs
# 
# This is free and unencumbered software released into the public domain.
###############################################################################

EXECUTABLES=fingerclient fingerserver
SOURCES:= $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-g -Wall -std=c++11

.PHONY: all
all: fingerserver fingerclient

fingerserver: fingerserver.cpp
	g++ $(CFLAGS) $< -o $@

fingerclient: fingerclient.cpp
	g++ $(CFLAGS) $< -o $@

# .INTERMEDIATE: %.o
%.o : %.cpp
	g++ $(CFLAGS) $< -o $@

clean: 
	rm -f fingerserver fingerclient $(OBJECTS)
