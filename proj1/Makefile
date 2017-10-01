CFLAGS=-g -Wall -std=c++11
SOURCES=lab1.cpp hangman.cpp
OBJECTS=$(SOURCES:.cpp=.o)

%.o : %.cpp
	g++ $(CFLAGS) $< -o $@

all: fingerserver fingerclient

fingerserver: fingerserver.cpp
	g++ $(CFLAGS) $< -o $@

fingerclient: fingerclient.cpp
	g++ $(CFLAGS) $< -o $@

clean: 
	rm fingerserver fingerclient $(OBJECTS)
