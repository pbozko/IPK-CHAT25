CXX = g++
CXXFLAGS = -g -std=c++20 -Wall -pedantic

TARGET = ipk25chat-client

SRCS = arg_parser.cpp error.cpp main.cpp
OBJS = arg_parser.o error.o main.o

all: $(TARGET)
	rm -f $(OBJS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

