#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <GL/glew.h>

#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include "shader_utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "res_texture.c"

struct TextureVertix {
  int vertix_index;
  int texture_index;
  int normal_index;

  friend std::istream& operator >> (std::istream& input, TextureVertix& vertix) {
    int vertix_index, texture_index, normal_index;
    while (input >> vertix_index >> texture_index >> normal_index) {
      // std::cout << vertix_index << " " << texture_index << " " << normal_index << std::endl;
      vertix.vertix_index = vertix_index;
      vertix.texture_index = texture_index;
      vertix.normal_index = normal_index;
    }
    return input;
  }
};

struct Side {
  std::vector<TextureVertix> texture_vertices;
};

struct OBJ {
  std::vector<GLfloat> vertices;
  std::vector<GLfloat> normal_vectors;
  std::vector<GLfloat> texture_coords;
  std::vector<Side> sides;
};

int screen_width = 800, screen_height = 600;
GLuint vbo_cube_vertices;
GLuint ibo_cube_elements;
GLuint program;

GLint attribute_coord3d;
GLint uniform_mvp;

GLuint vbo_cube_texcoords;
GLint attribute_texcoord;

GLuint texture_id;
GLint uniform_texture;

OBJ obj_res;

/**
 * Functions to transform sides with more than 3 vertices to triangles
 */

std::vector<Side> transform_side_to_triangles(const Side& side) {
  // Additional check, just in case
  if (side.texture_vertices.size() == 3) {
    return std::vector<Side>{ side };
  }
  std::vector<Side> new_sides;
  // Copy vertices to new variable
  auto vertices = side.texture_vertices;
  int missing_vertices = vertices.size() % 3;
  for (int i = 0; i < missing_vertices; i++) {
    vertices.push_back(vertices[i]);
  }
  std::cout << "Transformed Vertices: " << vertices.size() << std::endl;
  // Make sides grabbing groups of 3 vertices from `vertices`
  //
  return new_sides;
}

void transform_sides_to_triangles(OBJ *obj_res) {
  std::vector<Side> new_sides;
  for (auto& side : obj_res->sides) {
    if (side.texture_vertices.size() == 3) {
      new_sides.push_back(side);
    } else {
      auto aux_sides = transform_side_to_triangles(side);
      new_sides.insert(new_sides.end(), aux_sides.begin(), aux_sides.end());
    }
  }
}

/**
 * OBJ format reading util functions
 */
bool read_vertix(std::ifstream& input, OBJ *obj_res) {
  float x, y, z;
    input >> x >> y >> z;
    if (!input) {
      std::cerr << "Error reading vertices" << std::endl;
      return false;
    }
    obj_res->vertices.push_back(x);
    obj_res->vertices.push_back(y);
    obj_res->vertices.push_back(z);
    return true;
}

bool read_normal_vector(std::ifstream& input, OBJ *obj_res) {
  float x, y, z;
    input >> x >> y >> z;
    if (!input) {
      std::cerr << "Error reading normal vectors" << std::endl;
      return false;
    }
    obj_res->normal_vectors.push_back(x);
    obj_res->normal_vectors.push_back(y);
    obj_res->normal_vectors.push_back(z);
    return true;
}

bool read_texture_coordinates(std::ifstream& input, OBJ *obj_res) {
  float u, v, rest;
  input >> u >> v >> rest;
  if (!input) {
    std::cerr << "Error reading texture coordinates" << std::endl;
    return false;
  }
  obj_res->texture_coords.push_back(u);
  obj_res->texture_coords.push_back(v);
  return true;
}

bool read_side(std::ifstream& input, OBJ *obj_res) {
  Side side;
  std::string line;
  if (getline(input, line)) {
    std::istringstream line_input(line);
    // Skip line type
    std::string type;
    line_input >> type;
    TextureVertix vertix;
    while (line_input >> vertix) {
      side.texture_vertices.push_back(vertix);
    }
  }
  obj_res->sides.push_back(side);
  if (!input) {
    std::cerr << "Error reading sides" << std::endl;
    return false;
  }
  return true;
}

/**
 * Entry point to read OBJ file
 */
bool read_obj_file(const char *filename, OBJ *obj_res) {
  if (!filename) {
    std::cerr << "Must provide OBJ filename" << std::endl;
    return false;
  }

  if (!obj_res) {
    std::cerr << "Must provide valid OBJ resource" << std::endl;
    return false;
  }

  std::ifstream input(filename);
  std::string line_type;
  while (input >> line_type && line_type == "v") {
    if (!read_vertix(input, obj_res)) {
      return false;
    }
  }
  std::cout << "Vertices: " << obj_res->vertices.size() << std::endl;

  // line_type has next value, which should be "vn"
  if (line_type == "vn") {
    if (!read_normal_vector(input, obj_res)) {
      return false;
    }
  }

  while (input >> line_type && line_type == "vn") {
    if (!read_normal_vector(input, obj_res)) {
      return false;
    }
  }
  std::cout << "Normal vectors: " << obj_res->normal_vectors.size() << std::endl;

  if (line_type == "vt") {
    if (!read_texture_coordinates(input, obj_res)) {
      return false;
    }
  }
  while (input >> line_type && line_type == "vt") {
    if (!read_texture_coordinates(input, obj_res)) {
      return false;
    }
  }
  std::cout << "Texture coordinates: " << obj_res->texture_coords.size() << std::endl;

  if (line_type == "g" || line_type == "s") {
    input.ignore(1000, '\n');
  }

  while (input >> line_type && (line_type == "g" || line_type == "s" || line_type == "f")) {
    if (line_type == "g" || line_type == "s") {
      input.ignore(1000, '\n');
      continue;
    }
    // line_type == "f"
    if (!read_side(input, obj_res)) {
      return false;
    }
  }
  transform_sides_to_triangles(obj_res);
  std::cout << "Sides: " << obj_res->sides.size() << std::endl;

  return true;
}

int init_resources()
{
  if (!read_obj_file("House_3_AO.obj", &obj_res)) {
    std::cerr << "Error reading OBJ file" << std::endl;
    return false;
  }
  std::cout << "Read file correctly" << std::endl;

  // GLfloat cube_vertices[] = {
  //   // front
  //   -1.0, -1.0,  1.0,
  //    1.0, -1.0,  1.0,
  //    1.0,  1.0,  1.0,
  //   -1.0,  1.0,  1.0,
  //   // top
  //   -1.0,  1.0,  1.0,
  //    1.0,  1.0,  1.0,
  //    1.0,  1.0, -1.0,
  //   -1.0,  1.0, -1.0,
  //   // back
  //    1.0, -1.0, -1.0,
  //   -1.0, -1.0, -1.0,
  //   -1.0,  1.0, -1.0,
  //    1.0,  1.0, -1.0,
  //   // bottom
  //   -1.0, -1.0, -1.0,
  //    1.0, -1.0, -1.0,
  //    1.0, -1.0,  1.0,
  //   -1.0, -1.0,  1.0,
  //   // left
  //   -1.0, -1.0, -1.0,
  //   -1.0, -1.0,  1.0,
  //   -1.0,  1.0,  1.0,
  //   -1.0,  1.0, -1.0,
  //   // right
  //    1.0, -1.0,  1.0,
  //    1.0, -1.0, -1.0,
  //    1.0,  1.0, -1.0,
  //    1.0,  1.0,  1.0,
  // };

  glGenBuffers(1, &vbo_cube_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
  glBufferData(
    GL_ARRAY_BUFFER,
    obj_res.vertices.size() * sizeof(float),
    &obj_res.vertices[0],
    GL_STATIC_DRAW
  );

  // GLfloat cube_texcoords[48];

  // cube_texcoords[0] = 0.0;  cube_texcoords[1] = 0.0;
  // cube_texcoords[2] = 1.0;  cube_texcoords[3] = 0.0;
  // cube_texcoords[4] = 1.0;  cube_texcoords[5] = 1.0;
  // cube_texcoords[6] = 0.0;  cube_texcoords[7] = 1.0;

  // for(int i = 8; i < 48; i++){
  //   cube_texcoords[i] = cube_texcoords[i%8];
  // }

  glGenBuffers(1, &vbo_cube_texcoords);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
  glBufferData(
    GL_ARRAY_BUFFER,
    obj_res.texture_coords.size() * sizeof(float),
    &obj_res.texture_coords[0],
    GL_STATIC_DRAW
  );

  GLushort cube_elements[] = {
    // front
    0,  1,  2,
    2,  3,  0,
    // top
    4,  5,  6,
    6,  7,  4,
    // back
    8,  9, 10,
    10, 11,  8,
    // bottom
    12, 13, 14,
    14, 15, 12,
    // left
    16, 17, 18,
    18, 19, 16,
    // right
    20, 21, 22,
    22, 23, 20,
  };

  glGenBuffers(1, &ibo_cube_elements);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(cube_elements),
    cube_elements,
    GL_STATIC_DRAW
  );

  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    res_texture.width,
    res_texture.height,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    res_texture.pixel_data
  );


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
  glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
  glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);

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

void onKeyPress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: {
            glutLeaveMainLoop();
            break;
        }
    }
}

void free_resources()
{
  glDeleteProgram(program);
  glDeleteBuffers(1, &vbo_cube_vertices);
  glDeleteBuffers(1, &ibo_cube_elements);
  glDeleteBuffers(1, &vbo_cube_texcoords);
  glDeleteTextures(1, &texture_id);
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
    glutKeyboardFunc(onKeyPress);
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
