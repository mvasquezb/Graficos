#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_utils.h"

using namespace std;

GLuint vbo_vertices;

GLint program;

GLint attribute_coord2d;

// GLint uniform_t;
// float t = 0;

GLuint mvpMatrixId;
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;
glm::mat4 mvpMatrix;

bool initResources() {
	GLfloat vertices[] = {
		0.0, 0.0,
		0.5, 0.0,
		0.25, 0.5
	};

	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLint link_ok = GL_FALSE;
	GLuint vs, fs;
	if((vs = create_shader("basic.vertex.glsl", GL_VERTEX_SHADER))==0) return false;
	if((fs = create_shader("basic.fragment.glsl", GL_FRAGMENT_SHADER))==0) return false;

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
	
	// uniform_t = glGetUniformLocation(program, "t");
	// if (uniform_t == -1) {
	// 	cout << "Variable uniforme no especificada" << endl;
	// 	return false;
	// }

	mvpMatrixId = glGetUniformLocation(program, "mvp");
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	view = glm::lookAt(
		glm::vec3(0, 0, 1),	// Camera is at (4, 3, 3)
		glm::vec3(0, 0, 0), // Looking at origin
		glm::vec3(0, 1, 0)	// Head is up (change to (0, -1, 0) to look upside down)
	);

	// Model matrix: identity (no traslation or scaling by default)
	model = glm::mat4(1.0);

	// ModelViewProjection matrix
	// mvpMatrix = projection * view * model;
	mvpMatrix = projection * view * model;

	return true;
	
}

void onKeyPress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: { // Escape
			glutLeaveMainLoop();
			break;
		}
		case 'd': {
			model = glm::translate(model, glm::vec3(0.05, 0, 0));
			mvpMatrix = projection * view * model;
			glutPostRedisplay();
			break;
		}
	}
}

void onDisplay(){
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord2d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

	glVertexAttribPointer(attribute_coord2d,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,0);

	// glUniform1f(uniform_t, t);
	// Send our transformation to the currently bound shader, 
	// in the "mvp" uniform
	glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
		
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);

	glutSwapBuffers();
}

void freeResources(){
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_vertices);
}

int main(int argc, char* argv[]){
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

    if (initResources()) {
    	glutDisplayFunc(onDisplay);
    	glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glutKeyboardFunc(onKeyPress);
    	glutMainLoop();
    }

    freeResources();
    exit(EXIT_SUCCESS);
}