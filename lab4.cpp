#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdlib>
#include <vector>
#include <cstdio>
#include <iostream>

#include <GL/glew.h>

#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"
#include "SOIL.h"

#include "res_texture.c"

int screen_width=800, screen_height=600;
GLuint vbo_cube_vertices;
GLuint ibo_cube_elements;
GLuint program;

GLint attribute_coord3d;
GLint uniform_mvp;

GLuint vbo_cube_texcoords;
GLint attribute_texcoord;

GLuint texture_id;
GLint uniform_texture;

unsigned char* imag;

std::vector<float> 
      vertices_coords, // coordenadas de vertices para asignar a los puntos del objeto
      vertices, // coordenadas de vertices de tríangulos agrupadas cada 3
      uv_coord_vertices, // coordenadas uv para asignar luego a cada punto
      uv_coords; // coordenadas uv para cada punto de 'vertices'
std::vector<GLushort> indices_triangulos; // indices de cada coordenada uv para cada punto

void read_file(const char* filename){
    FILE* fp = fopen(filename, "rt");
    char buffer[100];

    
    
    // para pode usar los indices directamente
    // vertices.push_back(0.0);
    // vertices.push_back(0.0);
    // vertices.push_back(0.0);
    // uv_coords.push_back(0);
    // uv_coords.push_back(0);
    
    float x, y, z;
    
    fscanf(fp, "%s", buffer);
    while (strcmp(buffer, "v") == 0) {
      fscanf(fp, "%f %f %f", &x, &y, &z);
      vertices_coords.push_back(x);
      vertices_coords.push_back(y);
      vertices_coords.push_back(z);
      fscanf(fp, "%s", buffer);
    }

    while (strcmp(buffer, "vn") == 0) {
      fscanf(fp, "%f %f %f", &x, &y, &z);
      fscanf(fp, "%s", buffer);
    }

    while (strcmp(buffer, "vt") == 0) {
      fscanf(fp, "%f %f %f", &x, &y, &z);
      uv_coord_vertices.push_back(x);
      uv_coord_vertices.push_back(y);
      fscanf(fp, "%s", buffer);
    }


    int i_vertice, i_textura, i_normal;
    while (strcmp(buffer, "g") == 0 || strcmp(buffer, "s") == 0 || strcmp(buffer, "f") == 0) {
      if (strcmp(buffer, "g") == 0 || strcmp(buffer, "s") == 0) {
        fscanf(fp, "%s", buffer);
        fscanf(fp, "%s", buffer);
      } else {
        // leer datos de cada vertice
        std::vector<GLushort> indices;

        fscanf(fp, "%s", buffer);
        i_vertice = atoi(buffer);
        // si todavía hay datos de vertices
        while (i_vertice) {
          fscanf(fp, "%d %d", &i_textura, &i_normal);

          indices.push_back(i_vertice - 1);
          indices.push_back(i_textura - 1);

          if (fscanf(fp, "%s", buffer) == -1) {
            buffer[0] = 0;
          }

          i_vertice = atoi(buffer);
        }

        int n_vertices = indices.size() / 2;

        int i1 = vertices.size() / 3;
        GLushort i2 = i1 + 1 , i3 = i1 + 2 , i4 = i1 + 3 ;

        // por cada vertice de la cara
        for (int i = 0; i < n_vertices; i++) {
          // ingresar coordenadas de este vertice
          vertices.push_back(vertices_coords[indices[i * 2] * 3]); // x
          vertices.push_back(vertices_coords[indices[i * 2] * 3 + 1]); // y
          vertices.push_back(vertices_coords[indices[i * 2] * 3 + 2]); // z

          // ingresar coordenadas uv de este vertice
          uv_coords.push_back(uv_coord_vertices[indices[i * 2 + 1] * 2]);
          uv_coords.push_back(uv_coord_vertices[indices[i * 2 + 1] * 2 + 1]);

          if (i > 1) {
            //std::cout << "********************" << n_vertices << std::endl;;
          }
        }

        indices_triangulos.push_back(i1);
        indices_triangulos.push_back(i2);
        indices_triangulos.push_back(i3);

        if (n_vertices == 4) {
          indices_triangulos.push_back(i1);
          indices_triangulos.push_back(i3);
          indices_triangulos.push_back(i4);
        }
      }
    }

    fclose(fp);
}

void loadCube() {
  read_file("House_3_AO.obj");

  GLfloat* cube_vertices = new GLfloat[vertices.size()];
  for (long i = 0; i < vertices.size(); i++) {
    cube_vertices[i] = vertices[i];
    if (i < 15) {
      std::cout << cube_vertices[i] << std::endl;
    }
  }

  glGenBuffers(1, &vbo_cube_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), cube_vertices, GL_STATIC_DRAW);

  GLfloat* cube_texcoords = new GLfloat[uv_coords.size()];
  for (long i = 0; i < uv_coords.size(); i++) {
    cube_texcoords[i] = uv_coords[i];
  }

  glGenBuffers(1, &vbo_cube_texcoords);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uv_coords.size(),
               cube_texcoords, GL_STATIC_DRAW);

  GLushort* cube_elements = new GLushort[indices_triangulos.size()];
  for (long i = 0; i < indices_triangulos.size(); i++) {
    cube_elements[i] = indices_triangulos[i];
  }

  glGenBuffers(1, &ibo_cube_elements);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices_triangulos.size(), cube_elements, GL_STATIC_DRAW);




  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR);

  int width, height, channel;
  imag = SOIL_load_image("House_3_AO.png", &width, &height, &channel, SOIL_LOAD_AUTO);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               width,
               height,
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               imag);
}



int init_resources()
{
  loadCube();

  GLint link_ok = GL_FALSE;

  GLuint vs, fs;
  if ((vs = create_shader("cube.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
  if ((fs = create_shader("cube.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    fprintf(stderr, "glLinkProgram:");
    print_log(program);
    return 0;
  }

  const char* attribute_name;
  attribute_name = "coord3d";
  attribute_coord3d = glGetAttribLocation(program, attribute_name);
  if (attribute_coord3d == -1) {
    fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
    return 0;
  }

  attribute_texcoord = glGetAttribLocation(program,
                                           "texcoord");
  if(attribute_texcoord == -1){
    fprintf(stderr, "Could not bind attribute texcoord\n");
    return 0;
  }

  const char* uniform_name;
  uniform_name = "mvp";
  uniform_mvp = glGetUniformLocation(program, uniform_name);
  if (uniform_mvp == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }

  uniform_texture = glGetUniformLocation(program, "mytexture");
  if (uniform_texture == -1) {

    fprintf(stderr, "Could not bind uniform mytexture\n");
    return 0;
  }

  return 1;
}

void onIdle() {
  float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * glm::radians(15.0);  // base 15° per second
  glm::mat4 anim = \
    glm::rotate(glm::mat4(1.0f), angle*3.0f, glm::vec3(1, 0, 0)) *  // X axis
    glm::rotate(glm::mat4(1.0f), angle*2.0f, glm::vec3(0, 1, 0)) *  // Y axis
    glm::rotate(glm::mat4(1.0f), angle*4.0f, glm::vec3(0, 0, 1));   // Z axis

  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4.0));
  glm::mat4 view = glm::lookAt(glm::vec3(0.0, 15.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
  glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 100.0f);

  glm::mat4 mvp = projection * view * model * anim;
  glUseProgram(program);
  glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
  glutPostRedisplay();
}

void onDisplay()
{
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glUniform1i(uniform_texture, 0);

  glEnableVertexAttribArray(attribute_coord3d);
  // Describe our vertices array to OpenGL (it can't guess its format automatically)
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
  glVertexAttribPointer(
    attribute_coord3d, // attribute
    3,                 // number of elements per vertex, here (x,y,z)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is
    0,                 // no extra data between each position
    0                  // offset of first element
  );

  glEnableVertexAttribArray(attribute_texcoord);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);

  glVertexAttribPointer(
    attribute_texcoord,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,0
    );

  /* Push each element in buffer_vertices to the vertex shader */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
  int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
  glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(attribute_coord3d);
  glDisableVertexAttribArray(attribute_texcoord);
  glutSwapBuffers();
}

void onReshape(int width, int height) {
  screen_width = width;
  screen_height = height;
  glViewport(0, 0, screen_width, screen_height);
}

void free_resources()
{
  glDeleteProgram(program);
  glDeleteBuffers(1, &vbo_cube_vertices);
  glDeleteBuffers(1, &ibo_cube_elements);
  glDeleteBuffers(1, &vbo_cube_texcoords);
  glDeleteTextures(1, &texture_id);
  SOIL_free_image_data(imag);
}


int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitContextVersion(2,0);
  glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(screen_width, screen_height);
  glutCreateWindow("My Textured Cube");

  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
    return 1;
  }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
    return 1;
  }

  if (init_resources()) {
    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutIdleFunc(onIdle);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutMainLoop();
  }

  free_resources();
  return 0;
}
