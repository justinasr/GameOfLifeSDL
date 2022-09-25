CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -Llib -lSDL2main -lSDL2 -lSDL2_image
INCFLAGS = -Iinclude
 
main: main.o game.o
	g++ $(CXXFLAGS) -o game main.o game.o $(INCFLAGS) $(LDFLAGS)
 
main.o: main.cpp
	g++ $(CXXFLAGS) -o main.o -c main.cpp $(INCFLAGS) $(LDFLAGS)

game.o:
	g++ $(CXXFLAGS) -o game.o -c Game.cpp $(INCFLAGS) $(LDFLAGS)

clean:
	rm *.o game

all: clean main

.PHONY: main

