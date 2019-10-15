CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O3 -std=c++11

SRCS :=  print.cpp loader.cpp memory.cpp register.cpp controller.cpp main.cpp
OBJS := $(SRCS:%.cpp=%.o)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS)
