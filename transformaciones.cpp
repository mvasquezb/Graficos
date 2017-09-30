#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

GLuint program;
GLint attribute_coord;
GLint uniform_mvp;
GLint uniform_model;

//Variables para el movimiento
GLfloat alfa = 0.0,
        beta = 0.0,
        theta = 0.0,
        phi = 0.0,
        omega = 0.0;

glm::vec3 camera_position;

GLfloat t = 1.0f * glutGet(GLUT_ELAPSED_TIME);

int screen_width = 800,
    screen_height = 800;

struct Vertex{
    float x, y, z;
};

struct Triangle{
    unsigned int indices[3];
};

struct Mesh{
    //Informacion de estructura
    int numVertices;
    int numTriangles;
    Vertex* vertices;
    Triangle* triangles;

    //Información para transformación inicial
    Vertex center;
    float scale;

    //Matriz de transformación
    glm::mat4 model_transform;

    //Buffers para graficado
    GLfloat* object_vertices;
    GLushort* object_indexes;

    //Id's para buffers
    GLuint vbo_object;
    GLuint ibo_object;
};

struct Scene{
    int numMeshes;
    Mesh* meshes[5];
};


Scene scene;
int numEdges;

Mesh* readOFF(const char* filename) {
    FILE* fid = fopen(filename, "rt");

    //Leer formato
    char buffer[1024];
    fscanf(fid, "%s", buffer);

    if (strcmp(buffer, "OFF") != 0) {
        printf("Error de formato\n");
        exit(EXIT_FAILURE);
    }

    int nverts, ntriang, nedges;
    fscanf(fid, "%d %d %d", &nverts, &ntriang, &nedges);
    printf("%d, %d, %d\n", nverts, ntriang, nedges);

    Mesh* mesh = new Mesh;
    mesh->numVertices = nverts;
    mesh->numTriangles = ntriang;

    mesh->vertices = new Vertex[nverts];
    mesh->triangles = new Triangle[ntriang];
    mesh->center.x = 0.0;
    mesh->center.y = 0.0;
    mesh->center.z = 0.0;

    int i;
    for(i = 0; i < nverts; i++) {
        fscanf(fid, "%f %f %f", &mesh->vertices[i].x, &mesh->vertices[i].y, &mesh->vertices[i].z);
        mesh->center.x += mesh->vertices[i].x;
        mesh->center.y += mesh->vertices[i].y;
        mesh->center.z += mesh->vertices[i].z;
    }

    for(i = 0; i < ntriang; i++) {
        int nv;
        fscanf(fid, "%d %d %d %d", &nv, &mesh->triangles[i].indices[0],
                                        &mesh->triangles[i].indices[1],
                                        &mesh->triangles[i].indices[2]);
    }

    fclose(fid);
    mesh->center.x /= nverts;
    mesh->center.y /= nverts;
    mesh->center.z /= nverts;

    float maxx = -1.0e-10, maxy= -1.0e-10, maxz= -1.0e-10;
    float minx = 1.0e10, miny= 1.0e10, minz= 1.0e10;

    for(int i = 0; i < mesh->numVertices; i++) {
        if (mesh->vertices[i].x < minx)
            minx = mesh->vertices[i].x;
        if (mesh->vertices[i].x > maxx)
            maxx = mesh->vertices[i].x;
        if (mesh->vertices[i].y < miny)
            miny = mesh->vertices[i].y;
        if (mesh->vertices[i].y > maxy)
            maxy = mesh->vertices[i].y;
        if (mesh->vertices[i].z < minz)
            minz = mesh->vertices[i].z;
        if (mesh->vertices[i].z > maxz)
            maxz = mesh->vertices[i].z;
    }

    float diag = sqrt((maxx-minx)*(maxx-minx) + (maxy-miny)*(maxy-miny)+(maxz-minz)*(maxz-minz));
    mesh->scale = 2.0/diag;

    mesh->model_transform = glm::mat4(1.0f);
    return mesh;
}

void initBuffers(Mesh* mesh) {
    mesh->object_vertices = new GLfloat[mesh->numVertices * 3];
    //mesh->object_color = new GLfloat[mesh->numVertices * 3];
    mesh->object_indexes = new GLushort[mesh->numTriangles * 3];

    int i;

    for(i = 0; i < mesh->numVertices; i++) {
        mesh->object_vertices[3 * i] = mesh->vertices[i].x;
        mesh->object_vertices[3 * i + 1] = mesh->vertices[i].y;
        mesh->object_vertices[3 * i + 2] = mesh->vertices[i].z;

    }

    for(i = 0; i < mesh->numTriangles; i++) {
        mesh->object_indexes[3 * i] = mesh->triangles[i].indices[0];
        mesh->object_indexes[3 * i + 1] = mesh->triangles[i].indices[1];
        mesh->object_indexes[3 * i + 2] = mesh->triangles[i].indices[2];
    }

    glGenBuffers(1, &mesh->vbo_object);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_object);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(GLfloat), mesh->object_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->ibo_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numTriangles * 3 * sizeof(GLushort), mesh->object_indexes, GL_STATIC_DRAW);
}


bool initResources() {
    scene.meshes[0] = readOFF("sphere.off"); //Sol
    scene.meshes[1] = readOFF("sphere.off"); //Tierra
    scene.meshes[2] = readOFF("sphere.off"); //Luna
    scene.meshes[3] = readOFF("saturn.off"); // Saturn
    // scene.meshes[4] = readOFF("sphere.off"); //Luna
    // scene.meshes[5] = readOFF("sphere.off"); //Luna

    scene.numMeshes = 4;

    initBuffers(scene.meshes[0]);
    initBuffers(scene.meshes[1]);
    initBuffers(scene.meshes[2]);
    initBuffers(scene.meshes[3]);
    // initBuffers(scene.meshes[4]);

    GLint link_ok = GL_FALSE;
    GLuint vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER);
    GLuint fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER);

    if (!vs || !fs) {
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

    attribute_coord = glGetAttribLocation(program, "coord3d");
    if (attribute_coord == -1) {
        std::cout << "No se puede asociar el atributo coord" << std::endl;
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if (uniform_mvp == -1) {
        std::cout << "No se puede asociar el uniform mvp" << std::endl;
        return false;
    }

    uniform_model = glGetUniformLocation(program, "model");
    if (uniform_model == -1) {
        std::cout << "No se puede asociar el uniform model" << std::endl;
        return false;
    }

    return true;
}

void drawMesh(Mesh* mesh) {
    //Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = mesh->model_transform;
    
    glm::mat4 view  = glm::lookAt(camera_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 100.0f);
    // glm::mat4 projection = glm::ortho(-15.f, 15.f, -15.f, 15.f, 0.f, 15.f);
    glm::mat4 mvp = projection * view ;

    glUseProgram(program);

    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(uniform_model,1, GL_FALSE, glm::value_ptr(model));

    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_object);

    glVertexAttribPointer(
        attribute_coord,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, 0
    );

    //Dibujar las primitivas
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo_object);
    int size;   glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    //Dibujar los triánglos
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attribute_coord);
}

void onDisplay() {

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < scene.numMeshes; i++) {
        drawMesh(scene.meshes[i]);
    }

    glutSwapBuffers();
}

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;

    glViewport(0, 0, screen_width, screen_height);
}

void onKeyPress(unsigned char key, int x, int y) {
    switch(key) {
        case 27: {
            glutLeaveMainLoop();
            break;
        }
    }
}

void animate() {
    GLfloat t1 = 10.f * glutGet(GLUT_ELAPSED_TIME);
    GLfloat delta = t1 - t;
    // t = t1;

    delta = delta / 75.0f * glm::radians(1.0f);
    alfa = delta;
    beta = delta;
    theta = delta;
    phi = delta;
    omega += 0.1;
    
    camera_position = glm::rotate(glm::vec3(10.f), omega, glm::vec3(0, 1, 0));

    scene.meshes[0]->model_transform = glm::rotate(glm::mat4(1.0f), phi / 20, glm::vec3(0, 1, 0));

    scene.meshes[1]->model_transform =
        // glm::rotate(glm::mat4(1.0f), alfa, glm::vec3(0, 1, 0)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)) *
        // glm::rotate(glm::mat4(1.0f), beta, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));

    scene.meshes[2]->model_transform = 
        // glm::rotate(glm::mat4(1.0f), alfa, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)) *
        // glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
        // glm::rotate(glm::mat4(1.0f), phi, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    scene.meshes[3]->model_transform = 
        // glm::rotate(glm::mat4(1.0f), alfa / 5.0f, glm::vec3(0, 1, 0)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(8, 0, 2)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.4, 0.4, 0.4)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(40.0f), glm::vec3(1, 0, 1)) *
        // glm::rotate(glm::mat4(1.0f), beta / 2.0f, glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));

    // scene.meshes[4]->model_transform = 
    //     glm::translate(glm::mat4(1.0f), glm::vec3(6.0f - 1.0f * delta / 1000.f, 0, 0)) *
    //     glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    glutPostRedisplay();
}

void freeResources() {
    glDeleteProgram(program);

    for(int i = 0; i < scene.numMeshes; i++) {
        glDeleteBuffers(1, &scene.meshes[i]->vbo_object);
        glDeleteBuffers(1, &scene.meshes[i]->ibo_object);
        delete[] scene.meshes[i]->object_vertices;
        delete[] scene.meshes[i]->object_indexes;
        delete[] scene.meshes[i]->vertices;
        delete[] scene.meshes[i]->triangles;
        delete scene.meshes[i];
    }
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
        glutIdleFunc(animate);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    freeResources();
    exit(EXIT_SUCCESS);
}
