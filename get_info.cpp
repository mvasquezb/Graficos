#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void print(glm::mat4 matrix);

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(0, 0);
    glutCreateWindow("Nuevo OpenGL");

    GLenum glew_status = glewInit();
    if(glew_status != GLEW_OK){
        cout << "Error inicializando GLEW!" << endl;
        exit(EXIT_FAILURE);
    }

    if(!GLEW_VERSION_2_0){
        cout << "Su tajeta gráfica no soporta OpenGL 2.0" << endl;
        exit(EXIT_FAILURE);
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    cout << "GL Vendor:" << vendor << endl;
    cout << "GL Renderer:" << renderer << endl;
    cout << "GL Version (string):" << version << endl;
    cout << "GL Version (integer):" << major << ", " << minor << endl;
    cout << "GLSL Version:" << glslVersion << endl;

    // GLint nExtensions;
    // glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);

    // for(int i = 0; i < nExtensions; i++){
        // cout << glGetStringi(GL_EXTENSIONS, i) << endl;
    // }

	glm::mat4 matrixA = glm::mat4(1.0);
	print(matrixA);
	
	float v[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
	// glm::mat4 matrixB = glm::mat4();
	glm::mat4 matrixB = glm::make_mat4(v);
	print(matrixB);
	memcpy(glm::value_ptr(matrixB), v, sizeof(float) * 16);
	print(matrixB);
    exit(EXIT_SUCCESS);
}

void print(glm::mat4 matrix) {
	float *value_ptr = glm::value_ptr(matrix);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << *(value_ptr + i * 4 + j) << " ";
		}
		cout << endl;
	}
	cout << endl;
}