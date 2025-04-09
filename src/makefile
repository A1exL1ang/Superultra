# Variables
CXX = g++
CXXFLAGS = -std=c++17 -O3 -funroll-loops -ffast-math -ftree-vectorize -ftree-loop-vectorize -Wall -Wextra -flto -pthread -lpthread
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

# Example: "make ARCH=skylake-avx512"
ARCH ?= native
ARCHFLAGS = -march=$(ARCH) -mtune=$(ARCH)

# Append .exe, use del, and adjust stack size only if on Windows
ifeq ($(OS), Windows_NT)
	LDFLAGS = -Wl,--stack=8388608,--no-whole-archive -static
	NAME = Superultra-2.1.exe
	RM = del -f
else
	LDFLAGS = -Wl,--no-whole-archive -static
	NAME = Superultra-2.1
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
