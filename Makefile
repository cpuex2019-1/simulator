CXX = g++
PROG = mipsim
CXXFLAGS = -Wall -O -std=c++11

SRCS := main.cpp loader.cpp
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

$(PROG): $(OBJS)
	$(CXX) -o $@ $^

%.o: %.c
	$(CXX) -c -MMD -MP $<

clean:
	rm -f $(PROG) $(OBJS) $(DEPS)
