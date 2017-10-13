#include <iostream>
#include <fstream>
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

bool read_control_file(
    const char *filename, 
    std::vector<GLfloat>& control_points, 
    std::vector<GLushort>& control_patches
) {
    if (!filename) {
        std::cerr << "Must provide a filename" << std::endl;
        return false;
    }
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    int num_control_points, num_control_patches;
    input >> num_control_points >> num_control_patches;
    
    for (int i = 0; i < num_control_points && input && !input.eof(); i++) {
        float p1, p2, p3;
        input >> p1 >> p2 >> p3;
        if (!input) {
            std::cerr << "Error reading control points. Line: " << i + 1 << std::endl;
            break;
        }
        control_points.push_back(p1);
        control_points.push_back(p2);
        control_points.push_back(p3);
    }
    if (!input) {
        std::cerr << "Error reading control points" << std::endl;
        return false;
    }
    if (control_points.size() / 3 != num_control_points) {
        std::cerr << "Error reading control points. Size mismatch, expected " << num_control_points << ", got " << control_points.size() / 3 << std::endl;
        return false;
    }

    for (int i = 0; i < num_control_patches && input && !input.eof(); i++) {
        ushort p1, p2, p3, p4;
        for (int j = 0; j < 4; j++) {
            input >> p1 >> p2 >> p3 >> p4;
            if (!input) {
                std::cerr << "Error reading control patches. Line: " << i + 1 << std::endl;
                break;
            }
            control_patches.push_back(p1);
            control_patches.push_back(p2);
            control_patches.push_back(p3);
            control_patches.push_back(p4);
        }
    }

    if (!input) {
        std::cerr << "Error reading control patches" << std::endl;
        return false;
    }

    if (control_patches.size() / 16 != num_control_patches) {
        std::cerr << "Error reading control patches. Size mismatch, expected " << num_control_patches << ", got " << control_patches.size() / 16 << std::endl;
        return false;
    }

    return true;
}

bool init_resources() {

    /*EXAMEN: Asignar memoria y valores a surface_vertices y surface_color*/
    std::vector<GLfloat> control_points;
    std::vector<GLushort> control_patches;

    if (!read_control_file("control.txt", control_points, control_patches)) {
        std::cerr << "Error reading control file" << std::endl;
        return false;
    }
    std::cout << "DEBUG: " << control_points.size() / 3 << " " << control_patches.size() / 16 << std::endl;
    return false;

    glGenBuffers(1, &vbo_surface);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
    glBufferData(GL_ARRAY_BUFFER, 0/*EXAMEN: Tamaño de buffer*/, surface_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, 0/*EXAMEN: Tamaño de buffer*/, surface_color, GL_STATIC_DRAW);

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

    glDrawArrays(GL_POINTS, 0, 0/*EXAMEN: Numero de puntos*/);

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


    /*EXAMEN - OPCIONAL: elimine la memoria que se utilizó en los buffers. Use delete o free en los arreglos
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
