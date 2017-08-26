TARGET=get_info
LIBS=-L"C:\Users\alulab14.INF\Downloads\freeglut\lib" -lfreeglut  -L"C:\Users\alulab14.INF\Downloads\glew-1.13.0\src" -lglew -lglu32 -lopengl32
CFLAGS=-I"C:\Users\alulab14.INF\Downloads\freeglut\include" -I"C:\Users\alulab14.INF\Downloads\glew-1.13.0\include" -I"C:\Users\alulab14.INF\Downloads\glm-0.9.7.3\glm"
CSTD=-std=c++11

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = get_info.o
HEADERS =

%.o:	%.cpp	$(HEADERS)
	g++ -c $< -o $@ $(CFLAGS)


$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
