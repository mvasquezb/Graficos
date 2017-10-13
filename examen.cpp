#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <array>
#include <valarray>

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

/**
 * Global variables
 */
GLuint vbo_surface;
GLuint vbo_color;
GLint uniform_mvp;

GLuint program;

GLint attribute_coord3d;
GLint attribute_color;

GLfloat* surface_vertices;
GLfloat* surface_color;

int screen_width = 800, screen_height = 800;

int nSteps = 100;
int factorial[4] = {1, 1, 2, 6};

Surface *surface;

/**
 * End Global variables
 */

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

float binomial(int n, int i) {
    return 1.0 * factorial[n] / (factorial[i] * factorial[n - i]);
}

float bernstein(int n, int i, float t) {
    return binomial(n, i) * powf(t, i) * powf(1 - t, n - i);
}

void setupSurfaceControlMatrix(
    Surface *surface,
    const std::vector<GLfloat>& control_points,
    const std::vector<GLushort>& control_patch
) {
    float u = 0;
    for (int i = 0; i <= nSteps; i++) {
        float v = 0;
        for (int j = 0; j <= nSteps; j++) {
            float x = 0, y = 0, z = 0;

            for (int k = 0; k < 4; ++k) {
                float bi = bernstein(3, k, u);

                for (int l = 0; l < 4; ++l) {
                    float bj = bernstein(3, l, v);

                    int point_index = control_patch[k * 4 + l];
                    
                    x += bi * bj * control_points[point_index];
                    y += bi * bj * control_points[point_index + 1];
                    z += bi * bj * control_points[point_index + 2];
                }
            }

            surface->points.push_back(x);
            surface->points.push_back(y);
            surface->points.push_back(z);

            v += 1.0 / nSteps;
        }
        u += 1.0 / nSteps;
    }
}

void setupSurfacePoints(
    Surface *surface,
    const std::vector<GLfloat>& control_points, 
    const std::vector<GLushort>& control_patches
) {
    for (int i = 0; i < control_patches.size() / 16; i++) {
        auto start = control_patches.begin() + 16 * i;
        auto end = start + 16;
        auto indices = std::vector<GLushort>(start, end);
        setupSurfaceControlMatrix(surface, control_points, indices);
    }

    for (int i = 0; i < surface->points.size() / 3; ++i) {
        surface->colors.push_back(0.0);
        surface->colors.push_back(1.0);
        surface->colors.push_back(0.0);
    }
}

bool init_resources() {
    std::vector<GLfloat> control_points;
    std::vector<GLushort> control_patches;

    if (!read_control_file("control.txt", control_points, control_patches)) {
        std::cerr << "Error reading control file" << std::endl;
        return false;
    }

    surface = new Surface;

    setupSurfacePoints(surface, control_points, control_patches);

    surface->configBufferData();
    surface->configBufferColor();

    GLint link_ok = GL_FALSE;
    GLuint vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER);
    GLuint fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER);

    if(!vs || !fs) {
        return false;
    }

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
    switch (key) {
        case 27: {
            glutLeaveMainLoop();
            break;
        }
    }
}

void free_resources() {
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
    if(glew_status != GLEW_OK) {
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(!GLEW_VERSION_2_0) {
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(init_resources()) {
        glutKeyboardFunc(onKeyPress);
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
