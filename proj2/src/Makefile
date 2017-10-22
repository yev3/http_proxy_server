###############################################################################
# Project2, CPSC 5510 Networking, Seattle University
# Zach, David, Yevgeni Kamenski
# 
# Makefile - compiles the proxy program
# 
# This is free and unencumbered software released into the public domain.
###############################################################################

EXECUTABLES=proxy
SOURCES:= HttpRequest.cpp helpers.cpp proxy.cpp
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-g -Wall -std=c++11

.PHONY: all
all: proxy

proxy: $(OBJECTS)
	g++ $(CFLAGS) $(OBJECTS) -o $@

# .INTERMEDIATE: %.o
%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@

clean: 
	rm -f proxy $(OBJECTS)
