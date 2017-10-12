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

GLuint vbo_surface;
GLuint vbo_color;
GLint uniform_mvp;

GLuint program;

GLint attribute_coord3d;
GLint attribute_color;

GLfloat* surface_vertices;
GLfloat* surface_color;

int screen_width = 800, screen_height = 800;

bool init_resources() {

    /*EXAMEN: Asignar memoria y valores a surface_vertices y surface_color*/


    glGenBuffers(1, &vbo_surface);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
    glBufferData(GL_ARRAY_BUFFER, /*EXAMEN: Tama�o de buffer*/, surface_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, /*EXAMEN: Tama�o de buffer*/, surface_color, GL_STATIC_DRAW);

    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if((vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER))==0) return false;
    if((fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER))==0) return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if(!link_ok) {
        std::cout << "Problemas con el Shader" << std::endl;
        return false;
    }

    attribute_coord3d = glGetAttribLocation(program, "coord3d");
    if(attribute_coord3d == -1) {
        std::cout << "No se puede asociar el atributo coord3d" << std::endl;
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if(attribute_color == -1) {
        std::cout << "No se puede asociar el atributo color" << std::endl;
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if(uniform_mvp == -1) {
        std::cout << "No se puede asociar el uniform mvp" << std::endl;
        return false;
    }



    return true;
}

void onDisplay() {
    //Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1.5));
    glm::mat4 view  = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);
    glm::mat4 mvp = projection * view * model;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glEnableVertexAttribArray(attribute_coord3d);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);

    glVertexAttribPointer(
        attribute_coord3d,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, 0
    );

    glEnableVertexAttribArray(attribute_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);

    glVertexAttribPointer(
        attribute_color,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, 0
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);

    glDrawArrays(GL_POINTS, 0, /*EXAMEN: Numero de puntos*/);

    glDisableVertexAttribArray(attribute_coord3d);
    glDisableVertexAttribArray(attribute_color);
    glutSwapBuffers();
}

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;

    glViewport(0,0,screen_width, screen_height);
}

void free_resources() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_surface);
    glDeleteBuffers(1, &vbo_color);


    /*EXAMEN - OPCIONAL: elimine la memoria que se utiliz� en los buffers. Use delete o free en los arreglos
                surface_vertices y surface_color
    */
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if(glew_status != GLEW_OK) {
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(!GLEW_VERSION_2_0) {
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(init_resources()) {
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    free_resources();
    exit(EXIT_SUCCESS);
}
