CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O -std=c++11

SRCS := main.cpp loader.cpp memory.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS) $(DEPS)
