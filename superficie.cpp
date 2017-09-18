#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <utility>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

class Surface {
public:
    ~Surface() {
        clearBuffers();
    }
    void clearBuffers() {
        glDeleteBuffers(1, &vbo_surface);
        glDeleteBuffers(1, &vbo_color);
    }
    void bindDataTo(GLint attribId) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
        glVertexAttribPointer(
            attribId,
            3,
            GL_FLOAT,
            GL_FALSE,
            0, 0
        );
    }
    void bindColorTo(GLint attribId) {
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size(), &points[0], GL_STATIC_DRAW);
    }
    void configBufferColor() {
        glGenBuffers(1, &vbo_color);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * colors.size(), &colors[0], GL_STATIC_DRAW);
    }
    void drawArrays() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
        glDrawArrays(GL_LINE_STRIP, 0, points.size() / 3);
    }


    GLuint vbo_surface;
    GLuint vbo_color;
    std::vector<GLfloat> points;
    std::vector<GLfloat> colors;
    int numSteps;
    int numColors;
};


GLint uniform_mvp;

GLuint program;

GLint attribute_coord3d;
GLint attribute_color;

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
// int numPoints;
int factorial[4] = {1, 1, 2, 6};

Surface *surface;

float dx = 0, dy = 0, dz = 0;
float scale = 1;

float binomial(int n, int i) {
    return 1.0 * factorial[n] / (factorial[i] * factorial[n - i]);
}

float bernstein(int n, int i, float t) {
    return binomial(n, i) * powf(t, i) * powf(1 - t, n - i);
}

bool initResources(){
    surface = new Surface();

    float u = 0;

    for (int i = 0; i <= nSteps; i++) {
        float v = 0;
        for (int j = 0; j <= nSteps; j++) {
            float x = 0, y = 0, z = 0;

            for (int k = 0; k < 4; ++k) {
                float bi = bernstein(3, k, u);

                for (int l = 0; l < 4; ++l) {
                    float bj = bernstein(3, l, v);
                    x += bi * bj * cp[k][l][0];
                    y += bi * bj * cp[k][l][1];
                    z += bi * bj * cp[k][l][2];
                }
            }

            surface->points.push_back(x);
            surface->points.push_back(y);
            surface->points.push_back(z);

            v += 1.0/nSteps;
        }
        u += 1.0/nSteps;
    }

    for (int i = 0; i < surface->points.size() / 3; ++i) {
        surface->colors.push_back(0.0);
        surface->colors.push_back(1.0);
        surface->colors.push_back(0.0);
    }

    surface->configBufferData();
    surface->configBufferColor();

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
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    model = glm::translate(model, glm::vec3(dx, dy, dz));

    glm::mat4 view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f * screen_width / screen_height, 0.1f, 10.0f);
    glm::mat4 mvp = projection * view * model;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glEnableVertexAttribArray(attribute_coord3d);
    glEnableVertexAttribArray(attribute_color);

    surface->bindDataTo(attribute_coord3d);
    surface->bindColorTo(attribute_color);
    surface->drawArrays();

    glDisableVertexAttribArray(attribute_coord3d);
    glDisableVertexAttribArray(attribute_color);

    glutSwapBuffers();
    
}

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;

    glViewport(0,0,screen_width, screen_height);
}

void onKeyPress(unsigned char key, int x, int y) {
    bool redisplay = false;
    switch(key) {
        case 27: {
            glutLeaveMainLoop();
            break;
        }
        case 'W':
        case 'w': {
            dy += 0.01;
            redisplay = true;
            break;
        }
        case 'S':
        case 's': {
            dy -= 0.01;
            redisplay = true;
            break;
        }
        case 'D':
        case 'd': {
            dx += 0.01;
            redisplay = true;
            break;
        }
        case 'A':
        case 'a': {
            dx -= 0.01;
            redisplay = true;
            break;
        }
        case 'i': {
            scale *= 1.02;
            redisplay = true;
            break;
        }
        case 'j': {
            scale *= 0.98;
            redisplay = true;
            break;
        }
    }
    if (redisplay) {
        glutPostRedisplay();
    }
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
        glutKeyboardFunc(onKeyPress);
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
