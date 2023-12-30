# Variables
CXX = g++
CXXFLAGS = -std=c++17 -O3 -funroll-loops -ffast-math -ftree-vectorize -ftree-loop-vectorize -Wall -Wextra -flto -pthread -lpthread
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

# Example: "make ARCH=skylake-avx512"
ARCH ?= native
ARCHFLAGS = -march=$(ARCH) -mtune=$(ARCH)

# Note: stack size is adjusted here
LDFLAGS = -Wl,--stack=8589934592 -Wl,--no-whole-archive -static

# Append .exe and use del if on Windows
ifeq ($(OS), Windows_NT)
	NAME = Superultra.exe
	RM = del -f
else
	NAME = Superultra
	RM = rm -f
endif

# All .o ---> executable
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(ARCHFLAGS) $(LDFLAGS) -o $@ $^

# .cpp ---> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(ARCHFLAGS) -MMD -MP -c -o $@ $<

# Delete all .o, .d, and executable
.PHONY: clean
clean:
	$(RM) $(OBJ) $(DEP) $(NAME)

# Include .d files (the dep stuff is so that changes to .h are reflected)
-include $(DEP)
