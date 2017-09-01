#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

using namespace std;

GLuint vbo_vertices;

GLint program;

GLint attribute_coord2d;

bool init_resources() {
	GLfloat vertices[] = {
		0.0, 0.0,
		0.5, 0.0,
		0.25, 0.5
	};

	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLint link_ok = GL_FALSE;
	GLuint vertexShader, fragmentShader;
	
	vertexShader = create_shader("basic.v.glsl", GL_VERTEX_SHADER);
	fragmentShader = create_shader("basic.f.glsl", GL_FRAGMENT_SHADER);

	if (!vertexShader || !fragmentShader)
		return false;

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
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

	return true;
	
}

void onDisplay() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord2d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

	glVertexAttribPointer(
		attribute_coord2d, 	// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized ?
		0,					// stride (step)
		0					// array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);

	glutSwapBuffers();
}

void free_resources() {
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_vertices);
}

void onKeyPress(unsigned char key, int mouseX, int mouseY) {
	switch (key) {
		case 27: // Escape
			glutLeaveMainLoop();
			break;
	}
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(200, 200);
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
    	glutKeyboardFunc(onKeyPress);
    	glutDisplayFunc(onDisplay);
    	glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    	glutMainLoop();
    }

    exit(EXIT_SUCCESS);
}