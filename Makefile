CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O2 -std=c++11

SRCS :=  loader.cpp memory.cpp register.cpp controller.cpp main.cpp
OBJS := $(SRCS:%.cpp=%.o)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS)
