# Compiler
CXX = g++

# Source and header files
CODESRC = Barrier.cpp MapReduceFramework.cpp
HEADERS = Barrier.h MapReduceClient.h MapReduceFramework.h Utils.h

# Object files
OBJS = $(CODESRC:.cpp=.o)

# Static library name
LIBRARY = libMapReduceFramework.a

# Include paths and compiler flags
INCS = -I.
CXXFLAGS = -Wall -std=c++20 -O3 $(INCS)

# Tar settings
TAR = tar
TARFLAGS = -cvf
TARNAME = ex3.tar
TARSRCS = Barrier.cpp MapReduceFramework.cpp Barrier.h Makefile

# Default target
all: $(LIBRARY)

# Build object file
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build static library
$(LIBRARY): $(OBJS)
	ar rcs $@ $^

# Clean target
clean:
	rm -f $(LIBRARY) $(OBJS)

# Create tarball
tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

