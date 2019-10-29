CXX = g++
PROG = mipsim
PROG2 = mipsim-all
CXXFLAGS = -Wall -O3 -mtune=native -march=native -std=c++11 -fsanitize=address

SRCS :=  print.cpp loader.cpp memory.cpp register.cpp controller.cpp main.cpp
OBJS := $(SRCS:%.cpp=%.o)
SRCS2 :=  print.cpp loader.cpp memory.cpp register.cpp controller.cpp mipsim_all.cpp
OBJS2 := $(SRCS2:%.cpp=%.o)

all: $(PROG) $(PROG2)

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(PROG) $(OBJS)

$(PROG2): $(OBJS2)
	$(CXX) $(CXXFLAGS) -o $(PROG2) $(OBJS2)

clean:
	rm -f $(PROG) $(OBJS) $(PROG2) $(OBJS2)
