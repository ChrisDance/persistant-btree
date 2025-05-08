CC = g++
CFLAGS = -Wall -g -std=c++11 -Iinclude
LDFLAGS = 
TARGET = bin
OUTPUT_DIR = .

UNAME := $(shell uname)


SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $(OUTPUT_DIR)/$@ $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(TARGET) $(OBJECTS)


$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

.PHONY: all clean