TARGET=ta
BASE_DIR=/home/pmvb/Code/Graficos

CCSTD=c++11
CFLAGS=-std=$(CCSTD)

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = ta.o
HEADERS =

%.o:	%.cpp	$(HEADERS)
	g++ -c $< -o $@ $(CFLAGS)


$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) $(LIBS) -o $@

spheres: spheres.o
	g++ spheres.o $(LIBS) -o $@
	LD_LIBRARY_PATH=$(BASE_DIR) ./spheres

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f spheres

run: $(TARGET)
	LD_LIBRARY_PATH=$(BASE_DIR) ./$(TARGET)

optirun: $(TARGET)
	LD_LIBRARY_PATH=$(BASE_DIR) optirun ./$(TARGET)
