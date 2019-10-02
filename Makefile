CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O -std=c++11

SRCS := main.cpp controller.cpp loader.cpp memory.cpp register.cpp
OBJS := $(SRCS:%.cpp=%.o)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS)
