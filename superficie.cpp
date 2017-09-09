#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

class Surface {
public:
    Surface(int nSteps) {
        numSteps = nSteps;
        numPoints = 3 * (nSteps + 1);
        numColors = 3 * (nSteps + 1);
        points = new GLfloat[3 * (nSteps + 1)];
        colors = new GLfloat[3 * (nSteps + 1)];
    }
    ~Surface() {
        clearBuffers();
        delete[] this->points;
        delete[] this->colors;
    }
    void clearBuffers() {
        glDeleteBuffers(1, &vbo_surface);
        glDeleteBuffers(1, &vbo_color);
    }
    void bindDataTo(GLint attribId) {
        glEnableVertexAttribArray(attribId);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
        glVertexAttribPointer(
            attribId,
            2,
            GL_FLOAT,
            GL_FALSE,
            0, 0
        );
    }
    void bindColorTo(GLint attribId) {
        glEnableVertexAttribArray(attribId);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glVertexAttribPointer(
            attribId,
            3,
            GL_FLOAT,
            GL_FALSE,
            0, 0
        );
    }
    void configBufferData() {
        glGenBuffers(1, &vbo_surface);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numPoints, points, GL_STATIC_DRAW);
    }
    void configBufferColor() {
        glGenBuffers(1, &vbo_color);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numColors, colors, GL_STATIC_DRAW);
    }
    void drawArrays() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
        glDrawArrays(GL_LINE_STRIP, 0, numSteps + 1);
    }


    GLuint vbo_surface;
    GLuint vbo_color;
    GLfloat *points;
    GLfloat *colors;
    int numSteps;
    int numPoints;
    int numColors;
};


GLint uniform_mvp;

GLuint program;

GLint attribute_coord3d;
GLint attribute_color;

GLfloat* surface_vertices;
GLfloat* surface_color;

int screen_width = 800, screen_height = 800;

GLfloat cp[4][4][3] = {
    {{0.0, 0.0, 1.0},
     {0.0, 0.0, 0.66},
     {0.0, 0.0, 0.33},
     {0.0, 0.0, 0.0}},

    {{0.33, 0.0, 1.0},
     {0.33, 1.5, 0.66},
     {0.33, 1.5, 0.33},
     {0.33, 0.0, 0.0}},

    {{0.66, 0.0, 1.0},
     {0.66, 1.5, 0.66},
     {0.66, 1.5, 0.33},
     {0.66, 0.0, 0.0}},

    {{1.0, 0.0, 1.0},
     {1.0, 0.0, 0.66},
     {1.0, 0.0, 0.33},
     {1.0, 0.0, 0.0}}
};

int nSteps = 100;
int factorial[4] = {1, 1, 2, 6};

Surface *surface;

float binomial(int n, int i) {
    return factorial[n] / (float) (factorial[i] * factorial[n - i]);
}

float bernstein(int n, int i, float t) {
    return binomial(n, i) * std::powf(t, i) * std::powf(1 - t, n - i);
}

bool initResources(){
    surface = new Surface(nSteps);

    float u = 0;
    float v = 0;

    for (int i = 0; i <= nSteps; i++) {
        for (int j = 0; j <= nSteps; j++) {
            
        }
    }

	GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if((vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER)) == 0) {
        std::cout << "Problem with vertex shader" << std::endl;
        return false;
    }
    if((fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER)) == 0)  {
        std::cout << "Problem with fragment shader" << std::endl;
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cout << "Problemas con el Shader" << std::endl;
        return false;
    }

    attribute_coord3d = glGetAttribLocation(program, "coord3d");
    if (attribute_coord3d == -1) {
        std::cout << "No se puede asociar el atributo coord3d" << std::endl;
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if (attribute_color == -1) {
        std::cout << "No se puede asociar el atributo color" << std::endl;
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if (uniform_mvp == -1) {
        std::cout << "No se puede asociar el uniform mvp" << std::endl;
        return false;
    }

    return true;
}

void onDisplay(){
	//Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);
    glm::mat4 mvp = projection * view * model;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    

    glutSwapBuffers();
    
}

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;

    glViewport(0,0,screen_width, screen_height);
}

void freeResources(){
	glDeleteProgram(program);
    delete surface;
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_2_0) {
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (initResources()) {
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    freeResources();
    exit(EXIT_SUCCESS);
}
