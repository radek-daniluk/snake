LIBS=-lSDL2
CFLAGS= -std=gnu++14 -Wall -Wextra -Wpedantic
CXX = g++
snake: snake.cpp
	$(CXX) $^ -o snake $(LIBS)
