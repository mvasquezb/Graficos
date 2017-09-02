#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstdio>
#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

using namespace std;

GLuint program;
GLint attribute_coord;

GLint uniform_mvp;
GLint uniform_xmin;
GLint uniform_xmax;
GLint uniform_ymin;
GLint uniform_ymax;
GLint uniform_zmin;
GLint uniform_zmax;

int screen_width = 800, screen_height = 800;

#define MAXV 100000
#define MAXT 100000

float xmin = 1e09,
      xmax = -1e09,
      ymin = 1e09,
      ymax = -1e09,
      zmin = 1e09,
      zmax = -1e09;

GLuint vbo_object;
GLuint vbo_color;
GLuint ibo_object;

GLfloat vertices[MAXV][3];
GLushort triangles[MAXT][3];

int numVertices = 0;
int numTriangles = 0;

/**
 * Second object vars
 */

GLuint vbo_object2;
GLuint ibo_object2;

GLfloat vertices2[MAXV][3];
GLushort triangles2[MAXT][3];

int numVertices2 = 0;
int numTriangles2 = 0;
/**
 * End second object vars
 */

void read_off_file(const char* filename,
                   GLfloat vertices[MAXV][3],
                   GLushort triangles[MAXT][3],
                   int& numVertices,
                   int& numTriangles) {
    FILE* fp = fopen(filename, "rt");
    char buffer[100];

    //Leer formato
    fscanf(fp, "%s", buffer);
    int aux;

    //Leer #vertices, #triangulos, #aristas
    cout << numVertices << " " << numTriangles << endl;
    fscanf(fp, "%d %d %d", &numVertices, &numTriangles, &aux);
    cout << numVertices << " " << numTriangles << endl;
    for(int i = 0; i < numVertices; i++){
        fscanf(fp, "%f %f %f", &vertices[i][0], &vertices[i][1], &vertices[i][2]);
        xmin = min(xmin, vertices[i][0]);
        xmax = max(xmax, vertices[i][0]);
        
        ymin = min(ymin, vertices[i][1]);
        ymax = max(ymax, vertices[i][1]);

        zmin = min(zmin, vertices[i][2]);
        zmax = max(zmax, vertices[i][2]);
    }

    for(int i = 0; i < numTriangles; i++){
        fscanf(fp, "%d %d %d %d", &aux, &triangles[i][0], &triangles[i][1], &triangles[i][2]);
    }

    fclose(fp);
}


bool init_resources(){
    read_off_file("NR0.off", vertices, triangles, numVertices, numTriangles);

    glGenBuffers(1, &vbo_object);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_object);
    glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
        numTriangles * 3 * sizeof(GLushort),
		triangles,
		GL_STATIC_DRAW
	);

    /**
     * Second object
     */
    read_off_file("NR34.off", vertices2, triangles2, numVertices2, numTriangles2);

    glGenBuffers(1, &vbo_object2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object2);
    glBufferData(GL_ARRAY_BUFFER, numVertices2 * 3 * sizeof(GLfloat), vertices2, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo_object2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_object2);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        numTriangles2 * 3 * sizeof(GLushort),
        triangles2,
        GL_STATIC_DRAW
    );
    /**
     * End second object
     */

    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
	vs = create_shader("basic.v.glsl", GL_VERTEX_SHADER);
	fs = create_shader("basic.f.glsl", GL_FRAGMENT_SHADER);
	
    if(!vs || !fs)
		return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if(!link_ok){
        std::cout << "Problemas con el Shader" << std::endl;
        return false;
    }

    attribute_coord = glGetAttribLocation(program, "coord");
    if(attribute_coord == -1){
        std::cout << "No se puede asociar el atributo coord" << std::endl;
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if(uniform_mvp == -1){
        std::cout << "No se puede asociar el uniform mvp" << std::endl;
        return false;
    }


	uniform_xmax = glGetUniformLocation(program, "xmax");
    if(uniform_xmax == -1){
        std::cout << "No se puede asociar el uniform xmax" << std::endl;
        return false;
    }

    uniform_xmin = glGetUniformLocation(program, "xmin");
    if(uniform_xmin == -1){
        std::cout << "No se puede asociar el uniform xmin" << std::endl;
        return false;
    }

    uniform_ymax = glGetUniformLocation(program, "ymax");
    if(uniform_ymax == -1){
        std::cout << "No se puede asociar el uniform ymax" << std::endl;
        return false;
    }

    uniform_ymin = glGetUniformLocation(program, "ymin");
    if(uniform_ymin == -1){
        std::cout << "No se puede asociar el uniform ymin" << std::endl;
        return false;
    }

    uniform_zmax = glGetUniformLocation(program, "zmax");
    if(uniform_zmax == -1){
        std::cout << "No se puede asociar el uniform zmax" << std::endl;
        return false;
    }

    uniform_zmin = glGetUniformLocation(program, "zmin");
    if(uniform_zmin == -1){
        std::cout << "No se puede asociar el uniform zmin" << std::endl;
        return false;
    }
    return true;
}

void onDisplay(){
    //Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f * screen_width/screen_height, 0.1f, 10.0f);
    glm::mat4 mvp = projection * view * model;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform1f(uniform_xmax, xmax);
    glUniform1f(uniform_xmin, xmin);
    glUniform1f(uniform_ymax, ymax);
    glUniform1f(uniform_ymin, ymin);
    glUniform1f(uniform_zmax, zmax);
    glUniform1f(uniform_zmin, zmin);

    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object);

    glVertexAttribPointer(
        attribute_coord,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, 0
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_object);
    glDrawElements(GL_TRIANGLES, numTriangles * 3, GL_UNSIGNED_SHORT, 0);


    /**
     * Second object
     */
    glm::mat4 mvp2 = projection * view * glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 2.0f));
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));

    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object2);

    glVertexAttribPointer(
        attribute_coord,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, 0
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_object2);
    glDrawElements(GL_TRIANGLES, numTriangles2 * 3, GL_UNSIGNED_SHORT, 0);
    /**
     * End second object
     */


    glDisableVertexAttribArray(attribute_coord);
    glutSwapBuffers();
}

void onReshape(int w, int h){
    screen_width = w;
    screen_height = h;

    glViewport(0,0,screen_width, screen_height);
}

void free_resources(){
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_object);
    glDeleteBuffers(1, &ibo_object);
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if(glew_status != GLEW_OK){
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(!GLEW_VERSION_2_0){
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(init_resources()){
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
