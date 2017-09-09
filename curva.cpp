#include <iostream>
#include <cstdlib>
#include <cmath>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "shader_utils.h"

using namespace std;

GLuint program;

GLint attribute_coord2d;
GLint attribute_color;

//Puntos de control
GLfloat cp[4][2] = {
    {0.0, 0.0},
    {0.0, 0.5},
    {0.5, -0.5},
    {0.5, 0.0},
};

GLfloat curve_points2[4][2] = {
    {cp[3][0], cp[3][1]},
    {0.0, 0.0}, // To be set later
    {0.6, 0.6},
    {0.8, 0.0},
};

int nSteps = 100;

class Curve {
public:
    Curve(int nSteps) {
        this->points = new GLfloat[2 * (nSteps + 1)];
        this->colors = new GLfloat[3 * (nSteps + 1)];
    }
    ~Curve() {
        clearBuffers();
        delete[] this->points;
        delete[] this->colors;
    }
    void clearBuffers() {
        glDeleteBuffers(1, &vbo_curve);
        glDeleteBuffers(1, &vbo_color);
    }
    void bindDataTo(GLint attribId) {
        glEnableVertexAttribArray(attribId);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_curve);
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
        glGenBuffers(1, &vbo_curve);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_curve);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * (nSteps + 1), points, GL_STATIC_DRAW);
    }
    void configBufferColor() {
        glGenBuffers(1, &vbo_color);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * (nSteps + 1), colors, GL_STATIC_DRAW);
    }
    void drawArrays() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_curve);
        glDrawArrays(GL_LINE_STRIP, 0, nSteps + 1);
    }


    GLuint vbo_curve;
    GLuint vbo_color;
    GLfloat *points;
    GLfloat *colors;
};

Curve *curve1, *curve2;


float bezier3(float t, float p0, float p1, float p2, float p3) {
    float t1c = 1 - t;
    float t2c = t1c * t1c;
    float t3c = t1c * t2c;
    float t2 = t * t;
    float t3 = t * t2;

    return t3c * p0 + 3 * t2c * t * p1 + 3 * t1c * t2 * p2 + t3 * p3;
}

bool initResources() {
    curve1 = new Curve(nSteps);
    curve2 = new Curve(nSteps);

    float t = 0;

    // Calculate colinear point
    float d = 0.5;
    glm::vec2 p2 = glm::vec2(cp[2][0], cp[2][1]);
    glm::vec2 p3 = glm::vec2(cp[3][0], cp[2][3]);

    glm::vec2 diff_p3_p2 = p3 - p2;
    float norm_p3_p2 = distance2(p3, p2);
    std::cout << p4.x << " " << p4.y << std::endl;
    glm::vec2 p4 = p3 + d * diff_p3_p2 / norm_p3_p2;

    curve_points2[1][0] = p4.x;
    curve_points2[1][1] = p4.y;

    for (int i = 0; i <= nSteps; i++) {
        float px = bezier3(t, cp[0][0], cp[1][0], cp[2][0], cp[3][0]);
        float py = bezier3(t, cp[0][1], cp[1][1], cp[2][1], cp[3][1]);
        
        curve1->points[2 * i] = px;
        curve1->points[2 * i + 1] = py;

        px = bezier3(t, curve_points2[0][0], curve_points2[1][0], curve_points2[2][0], curve_points2[3][0]);
        py = bezier3(t, curve_points2[0][1], curve_points2[1][1], curve_points2[2][1], curve_points2[3][1]);
        curve2->points[2 * i] = px;
        curve2->points[2 * i + 1] = py;

        curve1->colors[3 * i] = curve2->colors[3 * i] = 1;
        curve1->colors[3 * i + 1] = curve2->colors[3 * i + 1] = 0;
        curve1->colors[3 * i + 2] = curve2->colors[3 * i + 2] = 0;

        t += 1.0 / nSteps;
    }

    curve1->configBufferData();
    curve1->configBufferColor();

    curve2->configBufferData();
    curve2->configBufferColor();

	GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if((vs = create_shader("basic.v.glsl", GL_VERTEX_SHADER)) == 0) {
        std::cout << "Problem with vertex shader" << std::endl;
        return false;
    }
    if((fs = create_shader("basic.f.glsl", GL_FRAGMENT_SHADER)) == 0)  {
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

    attribute_coord2d = glGetAttribLocation(program, "coord2d");
    if (attribute_coord2d == -1) {
        std::cout << "No se puede asociar el atributo coord2d" << std::endl;
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if (attribute_color == -1) {
        std::cout << "No se puede asociar el atributo color" << std::endl;
        return false;
    }

    return true;
}

void onDisplay() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    
    curve1->bindDataTo(attribute_coord2d);
    curve2->bindDataTo(attribute_coord2d);

    curve1->bindColorTo(attribute_color);
    curve2->bindColorTo(attribute_color);
    
    curve1->drawArrays();
    curve2->drawArrays();

    glDisableVertexAttribArray(attribute_coord2d);
    glDisableVertexAttribArray(attribute_color);

    glutSwapBuffers();
}

void onKeyPress(unsigned char key, int x, int y) {
    switch(key) {
        case 27: {
            // Escape
            glutLeaveMainLoop();
            break;
        }
    }
}

void freeResources() {
	glDeleteProgram(program);

    delete curve1;
    delete curve2;
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);
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
        glutKeyboardFunc(onKeyPress);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    freeResources();
    exit(EXIT_SUCCESS);
}
