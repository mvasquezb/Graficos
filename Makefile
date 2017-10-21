TARGET=cube_texture
BASE_DIR=/home/pmvb/Code/Graficos

FREEGLUT_LIB_DIR=$(BASE_DIR)/freeglut/lib
GLEW_LIB_DIR=$(BASE_DIR)/glew/lib

CCSTD=c++11
LIBS=-L"$(FREEGLUT_LIB_DIR)" -lfreeglut  -L"$(GLEW_LIB_DIR)" -lGLEW -lGLU -lGL
CFLAGS=-I"$(BASE_DIR)/freeglut/include" -I"$(BASE_DIR)/glew/include" -I"$(BASE_DIR)/glm" -I"$(BASE_DIR)/soil/src" -std=$(CCSTD)

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = cube_texture.o shader_utils.o image_DXT.o image_helper.o SOIL.o stb_image_aug.o
HEADERS =

%.o:	%.cpp	$(HEADERS)
	g++ -c $< -o $@ $(CFLAGS)


$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) $(LIBS) -o $@
	ln -sf $(FREEGLUT_LIB_DIR)/libfreeglut.so.3 .
	ln -sf $(GLEW_LIB_DIR)/libGLEW.so.1.13 .

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f libfreeglut.so.3
	-rm -f libGLEW.so.1.13

run: $(TARGET)
	LD_LIBRARY_PATH=$(BASE_DIR) ./$(TARGET)

optirun: $(TARGET)
	LD_LIBRARY_PATH=$(BASE_DIR) optirun ./$(TARGET)
