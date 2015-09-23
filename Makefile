# Make sure ROOT is available
ifeq ($(shell which root-config), )
  $(error ROOT installation is not found)
endif


# Compiler and linker flags
CC = c++
INCLUDE = -Iinclude/ -I$(shell root-config --incdir)
OPFLAGS = -O2
CFLAGS = -Wall -Wextra -fPIC -std=c++11 $(INCLUDE) $(OPFLAGS)


# Sources, object files, and their location
SOURCES = $(shell ls src/ | grep .cpp)
OBJECTS = $(SOURCES:.cpp=.o)
vpath %.cpp src/


# Phony targets
.PHONY: clean


# Default target
all: libHepPlotUtils.so


libHepPlotUtils.so: $(OBJECTS)
	@ mkdir -p lib/
	@ rm -f lib/$@
	@ $(CC) -shared -Wl,-install_name,$@.1 -o $@.1.0 $+
	@ mv $@.1.0 lib/
	@ ln -sf $@.1.0 lib/$@

%.o: %.cpp
	@ $(CC) $(CFLAGS) -c $< -o $@


clean:
	@ rm -f *.o
