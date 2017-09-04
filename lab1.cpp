#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

using namespace std;

GLint program;

GLuint vbo_vertices;
GLuint vbo_colors;

GLint attribute_coord2d;
GLint attribute_color;

GLint uniform_t;

float t = 0.0;

bool init_resources() {
    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if((vs = create_shader("basic.v.glsl", GL_VERTEX_SHADER))==0) return false;
    if((fs = create_shader("basic.f.glsl", GL_FRAGMENT_SHADER))==0) return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        cout << "Problemas con el shader" << endl;
        return false;
    }

    attribute_coord2d = glGetAttribLocation(program, "coord2d");
    if (attribute_coord2d == -1) {
        cout << "Variable de coordenadas no especificada" << endl;
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if (attribute_color == -1) {
        cout << "Variable de color no especificada" << endl;
        return false;
    }

    uniform_t = glGetUniformLocation(program, "t");
    if (uniform_t == -1) {
        cout << "Constante t no especificada" << endl;
        return false;
    }

    GLfloat vertices[] = {
        // Center rectangle
        -0.25, -0.5,
        0.25, -0.5,
        0.25, 0.5,
        -0.25, 0.5,

        // Left rectangle
        -0.4, -0.5,
        -0.9, -0.5,
        -0.9, 0.5,
        -0.4, 0.5,

        // Right rectangle
        0.4, -0.5,
        0.9, -0.5,
        0.9, 0.5,
        0.4, 0.5,
    };

    GLfloat colors[] = {
        // Red
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,

        // Green
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,

        // Blue
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
    };

    glGenBuffers(1, &vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    return true;
    
}

void keyboard(unsigned char key, int x, int y){
    switch (key) {
        case 27: { // Escape
            glutLeaveMainLoop();
            break;
        }
        case 'd':
        case 'D': {
            t += 0.05;
            glutPostRedisplay();
            break;
        }
        case 'a':
        case 'A': {
            t -= 0.05;
            glutPostRedisplay();
            break;
        }
    }
}

void onDisplay() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glEnableVertexAttribArray(attribute_coord2d);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

    glVertexAttribPointer(
        attribute_coord2d,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,0
    );

    glEnableVertexAttribArray(attribute_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glVertexAttribPointer(attribute_color,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,0
    );

    glUniform1f(uniform_t, t);

    // Changed draw mode from GL_TRIANGLES to GL_POLYGON, and set offsets to read from
    glDrawArrays(GL_POLYGON, 0, 4);
    glDrawArrays(GL_POLYGON, 4, 4);
    glDrawArrays(GL_POLYGON, 8, 4);
    glDisableVertexAttribArray(attribute_coord2d);
    glDisableVertexAttribArray(attribute_color);

    glutSwapBuffers();
}

void free_resources(){
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_vertices);
    glDeleteBuffers(1, &vbo_colors);
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutCreateWindow("Nuevo OpenGL");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        cout << "Error inicializando GLEW!" << endl;
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_2_0) {
        cout << "Su tajeta gráfica no soporta OpenGL 2.0" << endl;
        exit(EXIT_FAILURE);
    }

    if (init_resources()) {
        glutDisplayFunc(onDisplay);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutKeyboardFunc(keyboard);
        glutMainLoop();
    }

    exit(EXIT_SUCCESS);
}