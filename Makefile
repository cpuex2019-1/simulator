CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O -std=c++11

SRCS := main.cpp loader.cpp memory.cpp
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS) $(DEPS)
