TARGET=baseRep
BASE_DIR=C:/Users/alulab14.INF/Downloads/.Code/Graficos
LIBS=-L"$(BASE_DIR)/freeglut/lib" -lfreeglut  -L"$(BASE_DIR)/glew/src" -lglew -lglu32 -lopengl32
CFLAGS=-I"$(BASE_DIR)/freeglut/include" -I"$(BASE_DIR)/glew/include" -I"$(BASE_DIR)/glm"

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = baseRep.o shader_utils.o
HEADERS =

%.o:	%.cpp $(HEADERS)
	g++ $(CFLAGS) -c $< -o $@

$(TARGET):	$(OBJECTS)
	g++ $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
