#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <array>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        phi = 0.0;

GLfloat t = 1.0f * glutGet(GLUT_ELAPSED_TIME);

int screen_width = 800,
    screen_height = 800;

struct Vertex {
    float x, y, z;
};

struct Triangle {
    unsigned int indices[3];
};

struct Mesh {

    ~Mesh() {
        glDeleteBuffers(1, &vbo_object);
        glDeleteBuffers(1, &ibo_object);
        delete[] object_vertices;
        delete[] object_indexes;
        delete[] vertices;
        delete[] triangles;
    }

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

struct Scene {
    ~Scene() {
        for (auto& plane : planes) {
            delete plane;
        }

        for (auto& propeller : propellers) {
            delete propeller;
        }
    }

    int numMeshes;
    std::array<Mesh*, 5> planes;
    std::array<Mesh*, 5> propellers;
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

/**
 * Change initial plane positions
 */
bool initResources() {
    scene.planes[0] = readOFF("avion.off");
    scene.planes[1] = readOFF("avion.off");
    scene.planes[2] = readOFF("avion.off");
    scene.planes[3] = readOFF("avion.off");
    scene.planes[4] = readOFF("avion.off");
    
    scene.propellers[0] = readOFF("helice.off");
    scene.propellers[1] = readOFF("helice.off");
    scene.propellers[2] = readOFF("helice.off");
    scene.propellers[3] = readOFF("helice.off");
    scene.propellers[4] = readOFF("helice.off");

    for (auto& plane : scene.planes) {
        initBuffers(plane);
        plane->model_transform = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f, -6.f, -5.f));
    }

    // Setup initial plane positions
    // Mid-right plane
    scene.planes[1]->model_transform = glm::translate(
        scene.planes[1]->model_transform,
        glm::vec3(10.f, 1.f, 10.f)
    );
    // Center plane
    scene.planes[2]->model_transform = glm::translate(
        scene.planes[2]->model_transform,
        glm::vec3(-10.f, 1.f, -10.f)
    );
    // Mid-left plane
    scene.planes[3]->model_transform = glm::translate(
        scene.planes[3]->model_transform,
        glm::vec3(-18.f, 1.f, 0.f)
    );
    // Bottom-left plane.
    scene.planes[4]->model_transform = glm::translate(
        scene.planes[4]->model_transform,
        glm::vec3(-20.f, 1.f, 12.f)
    );


    for (auto& propeller : scene.propellers) {
        initBuffers(propeller);
        propeller->model_transform = glm::translate(glm::mat4(1.0f), glm::vec3(-15.f, -6.f, -15.f));
    }

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

    glm::mat4 view  = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f * screen_width / screen_height, 0.1f, 100.0f);
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
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    //Dibujar los triánglos
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attribute_coord);
}

void onDisplay() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& plane : scene.planes) {
        drawMesh(plane);
    }

    for (auto& propeller : scene.propellers) {
        drawMesh(propeller);
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

    delta = delta / 5.0f * glm::radians(1.0f);
    alfa = delta;

    glm::mat4 plane_transform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -0.1f));

    for (int i = 0; i < scene.planes.size(); ++i) {
        auto& plane = scene.planes[i];
        auto& propeller = scene.propellers[i];

        plane->model_transform = plane_transform * plane->model_transform;
        propeller->model_transform = 
            plane->model_transform *
            glm::rotate(glm::mat4(1.f), alfa, glm::vec3(0.f, 0.f, 1.f));
    }

    glutPostRedisplay();
}

void freeResources() {
    glDeleteProgram(program);
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
