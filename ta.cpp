
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

template <typename T> class Vec3 {
public:
  T x, y, z;

  Vec3() : x(T(0)), y(T(0)), z(T(0)) {}

  Vec3(T xx) : x(xx), y(xx), z(xx) {}

  Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

  Vec3 &normalize() {
    T nor2 = length2();
    if (nor2 > 0) {
      T invNor = 1 / sqrt(nor2);
      x *= invNor, y *= invNor, z *= invNor;
    }
    return *this;
  }

  Vec3<T> operator*(const T &f) const { return Vec3<T>(x * f, y * f, z * f); }

  Vec3<T> operator*(const Vec3<T> &v) const {
    return Vec3<T>(x * v.x, y * v.y, z * v.z);
  }

  //[/comment]
  // Added: Cross product method
  //[/comment]
  Vec3<T> cross(const Vec3<T> &v) const {
    return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }

  Vec3<T> operator-(const Vec3<T> &v) const {
    return Vec3<T>(x - v.x, y - v.y, z - v.z);
  }

  Vec3<T> operator+(const Vec3<T> &v) const {
    return Vec3<T>(x + v.x, y + v.y, z + v.z);
  }

  Vec3<T> &operator+=(const Vec3<T> &v) {
    x += v.x, y += v.y, z += v.z;
    return *this;
  }

  Vec3<T> &operator*=(const Vec3<T> &v) {
    x *= v.x, y *= v.y, z *= v.z;
    return *this;
  }
  //[comment]
  // Added: Allow division by scalar
  //[/comment]
  Vec3<T> &operator/=(const T &v) {
    x /= v, y /= v, z /= v;
    return *this;
  }

  Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }

  T length2() const { return x * x + y * y + z * z; }

  T length() const { return sqrt(length2()); }

  friend std::ostream &operator<<(std::ostream &os, const Vec3<T> &v) {
    os << "[" << v.x << " " << v.y << " " << v.z << "]";
    return os;
  }
};

typedef Vec3<float> Vec3f;

class Sphere {
public:
  Vec3f center;                      /// position of the sphere
  float radius, radius2;             /// sphere radius and radius^2
  Vec3f surfaceColor, emissionColor; /// surface color and emission (light)
  float transparency, reflection;    /// surface transparency and reflectivity
  Sphere(const Vec3f &c, const float &r, const Vec3f &sc, const float &refl = 0,
         const float &transp = 0, const Vec3f &ec = 0)
      : center(c), radius(r), radius2(r * r), surfaceColor(sc),
        emissionColor(ec), transparency(transp), reflection(refl) { /* empty */
  }
  //[comment]
  // Compute a ray-sphere intersection using the geometric solution
  //[/comment]
  bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0,
                 float &t1) const {
    Vec3f l = center - rayorig;
    float tca = l.dot(raydir);
    if (tca < 0)
      return false;
    float d2 = l.dot(l) - tca * tca;
    if (d2 > radius2)
      return false;
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    return true;
  }
};

class Vertex : public Vec3f {
public:
    Vec3f normal;
    int numTA;
};

class Triangle {
public:
  std::array<Vec3f, 3> vertices;

  Triangle(): vertices() {}
  Triangle(Vec3f p0, Vec3f p1, Vec3f p2) : vertices{p0, p1, p2} {}

  //[comment]
  // Compute ray-triangle intersection
  //[/comment]
  bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float& t) const {
    float area2 = normal.length();

    //[comment]
    // Find point of intersection with plane
    //[/comment]

    //[comment]
    // Check if ray and plane are parallel
    //[/comment]
    float nDotRay = normal.dot(raydir);
    if (nDotRay == 0.0) { // Use epsilon
      return false;
    }

    float d = normal.dot(vertices[0]);
    t = (normal.dot(rayorig) + d) / nDotRay;
    if (t < 0) {
      return false;
    }

    Vec3f P = orig + t * dir;

    // Inside-outside test
    Vec3f C;

    // Edge 0
    auto edge0 = vertices[1] - vertices[0];
    auto vp0 = P - vertices[0];
    C = edge0.cross(vp0);
    if (normal.dot(C) < 0) {
      return false;
    }

    // Edge 1
    auto edge1 = vertices[2] - vertices[1];
    auto vp1 = P - vertices[1];
    C = edge1.cross(vp1);
    if (normal.dot(C) < 0) {
      return false;
    }

    // Edge 2
    auto edge2 = vertices[0] - vertices[2];
    auto vp2 = P - vertices[2];
    C = edge2.cross(vp2);
    if (normal.dot(C) < 0) {
      return false;
    }

    return true;
  }
}

class Face {
public:
    unsigned int indices[3];
    Vec3f normal;
};

class Mesh {
public:
    //Informacion de estructura
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    //Información para transformación inicial
    Vertex center;
    float scale;

    //Matriz de transformación
    // glm::mat4 model_transform;

    // //Buffers para graficado
    // GLfloat* object_vertices;
    // GLfloat* object_normal;
    // GLushort* object_indexes;

    // //Id's para buffers
    // GLuint vbo_object;
    // GLuint vbo_normal;
    // GLuint ibo_object;

    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0,
                   float &t1) const {
      unsigned triIndex;
      bool intersects = false;
      for (int i = 0; i < faces.size(); i++) {
        const auto& face = faces[i];
        Triangle triangle(face.indices[0], face.indices[1], face.indices[2]);
        float tFront = INFINITY, tBack;
        if (triangle.intersect(rayorig, raydir, tFront) && tFront < t0) {
          t0 = tFront;
          triIndex = i;
          intersects |= true;
        }
      }
      return intersects;
    }

    static Mesh fromOFF(const char *filename) {
      FILE* fid = fopen(filename, "rt");

      //Leer formato
      char buffer[1024];
      fscanf(fid, "%s", buffer);

      if(strcmp(buffer, "OFF")!=0){
        printf("Error de formato\n");
        exit(EXIT_FAILURE);
      }

      int nverts, ntriang, nedges;
      fscanf(fid, "%d %d %d", &nverts, &ntriang, &nedges);
      printf("%d, %d, %d\n", nverts, ntriang, nedges);

      Mesh mesh;

      for(int i = 0; i < nverts; i++){
        Vertex vertex;
        fscanf(fid, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);

        mesh.center += vertex;
        mesh.vertices.push_back(vertex);
      }

      for(int i = 0; i < ntriang; i++) {
        Face face;
        int nv;
        fscanf(fid, "%d %d %d %d", &nv, &face.indices[0],
                                        &face.indices[1],
                                        &face.indices[2]);
        mesh.faces.push_back(face);
      }

      fclose(fid);
      mesh.center /= nverts;

      float maxx = -1.0e-10, maxy= -1.0e-10, maxz= -1.0e-10;
      float minx = 1.0e10, miny= 1.0e10, minz= 1.0e10;

      for(int i = 0; i < mesh.vertices.size(); i++){
        if(mesh.vertices[i].x < minx)
          minx = mesh.vertices[i].x;
        if(mesh.vertices[i].x > maxx)
          maxx = mesh.vertices[i].x;
        if(mesh.vertices[i].y < miny)
          miny = mesh.vertices[i].y;
        if(mesh.vertices[i].y > maxy)
          maxy = mesh.vertices[i].y;
        if(mesh.vertices[i].z < minz)
          minz = mesh.vertices[i].z;
        if(mesh.vertices[i].z > maxz)
          maxz = mesh.vertices[i].z;
      }

      unsigned int p0, p1, p2;
      //Computar normales de los triángulos
      for(int i = 0; i < mesh.faces.size(); i++){
        p0 = mesh.faces[i].indices[0];
        p1 = mesh.faces[i].indices[1];
        p2 = mesh.faces[i].indices[2];

        Vec3f A = mesh.vertices[p1] - mesh.vertices[p0];
        Vec3f B = mesh.vertices[p2] - mesh.vertices[p0];

        Vec3f C = A.cross(B);
        mesh.faces[i].normal = C;

        C.normalize();
        mesh.vertices[p0].normal += C;   mesh.vertices[p0].numTA++;
        mesh.vertices[p1].normal += C;   mesh.vertices[p1].numTA++;
        mesh.vertices[p2].normal += C;   mesh.vertices[p2].numTA++;
      }

      for(int i = 0; i < mesh.vertices.size(); i++){
        mesh.vertices[i].normal /= mesh.vertices[i].numTA;
        mesh.vertices[i].normal.normalize();
      }

      float diag = sqrt((maxx-minx)*(maxx-minx) +
                        (maxy-miny)*(maxy-miny)+
                        (maxz-minz)*(maxz-minz));
      mesh.scale = 2.0/diag;

      return mesh;
    }
};

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5

float mix(const float &a, const float &b, const float &mix) {
  return b * mix + a * (1 - mix);
}

//[comment]
// This is the main trace function. It takes a ray as argument (defined by its
// origin
// and direction). We test if this ray intersects any of the geometry in the
// scene.
// If the ray intersects an object, we compute the intersection point, the
// normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective,
// diffuse).
// The function returns a color for the ray. If the ray intersects an object
// that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
//[/comment]
Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir,
            const std::vector<Sphere> &spheres, const int &depth) {
  // if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
  float tnear = INFINITY;
  const Sphere *sphere = NULL;
  // find intersection of this ray with the sphere in the scene
  for (unsigned i = 0; i < spheres.size(); ++i) {
    float t0 = INFINITY, t1 = INFINITY;
    if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
      if (t0 < 0)
        t0 = t1;
      if (t0 < tnear) {
        tnear = t0;
        sphere = &spheres[i];
      }
    }
  }
  // if there's no intersection return black or background color
  if (!sphere)
    return Vec3f(2);
  // color of the ray/surfaceof the object intersected by the ray
  Vec3f surfaceColor = 0;
  Vec3f phit = rayorig + raydir * tnear; // point of intersection
  Vec3f nhit = phit - sphere->center;    // normal at the intersection point
  nhit.normalize();                      // normalize normal direction
  // If the normal and the view direction are not opposite to each other
  // reverse the normal direction. That also means we are inside the sphere so
  // set
  // the inside bool to true. Finally reverse the sign of IdotN which we want
  // positive.
  float bias = 1e-4; // add some bias to the point from which we will be tracing
  bool inside = false;
  if (raydir.dot(nhit) > 0)
    nhit = -nhit, inside = true;
  if ((sphere->transparency > 0 || sphere->reflection > 0) &&
      depth < MAX_RAY_DEPTH) {
    float facingratio = -raydir.dot(nhit);
    // change the mix value to tweak the effect
    float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
    // compute reflection direction (not need to normalize because all vectors
    // are already normalized)
    Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
    refldir.normalize();
    Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
    Vec3f refraction = 0;
    // if the sphere is also transparent compute refraction ray (transmission)
    if (sphere->transparency) {
      float ior = 1.1,
            eta = (inside) ? ior
                           : 1 / ior; // are we inside or outside the surface?
      float cosi = -nhit.dot(raydir);
      float k = 1 - eta * eta * (1 - cosi * cosi);
      Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
      refrdir.normalize();
      refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1);
    }
    // the result is a mix of reflection and refraction (if the sphere is
    // transparent)
    surfaceColor = (reflection * fresneleffect +
                    refraction * (1 - fresneleffect) * sphere->transparency) *
                   sphere->surfaceColor;
  } else {
    // it's a diffuse object, no need to raytrace any further
    for (unsigned i = 0; i < spheres.size(); ++i) {
      if (spheres[i].emissionColor.x > 0) {
        // this is a light
        Vec3f transmission = 1;
        Vec3f lightDirection = spheres[i].center - phit;
        lightDirection.normalize();
        for (unsigned j = 0; j < spheres.size(); ++j) {
          if (i != j) {
            float t0, t1;
            if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0,
                                     t1)) {
              transmission = 0;
              break;
            }
          }
        }
        surfaceColor += sphere->surfaceColor * transmission *
                        std::max(float(0), nhit.dot(lightDirection)) *
                        spheres[i].emissionColor;
      }
    }
  }

  return surfaceColor + sphere->emissionColor;
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of
// the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const std::vector<Sphere> &spheres) {
  unsigned width = 640, height = 480;
  Vec3f *image = new Vec3f[width * height],
        *pixel = image;
  float invWidth = 1 / float(width), invHeight = 1 / float(height);
  float fov = 30, aspectratio = width / float(height);
  float angle = tan(M_PI * 0.5 * fov / 180.);
  // Trace rays
  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x, ++pixel) {
      float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
      float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
      Vec3f raydir(xx, yy, -1);
      raydir.normalize();
      *pixel = trace(Vec3f(0), raydir, spheres, 0);
    }
  }
  // Save result to a PPM image (keep these flags if you compile under Windows)
  std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (unsigned i = 0; i < width * height; ++i) {
    ofs << (unsigned char)(std::min(float(1), image[i].x) * 255)
        << (unsigned char)(std::min(float(1), image[i].y) * 255)
        << (unsigned char)(std::min(float(1), image[i].z) * 255);
  }
  ofs.close();
  delete[] image;
}

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is
// complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv) {
  // Mesh mesh = Mesh::fromOFF("NR0.off");
  // srand48(13);
  std::vector<Sphere> spheres;
  // position, radius, surface color, reflectivity, transparency, emission color
  spheres.push_back(
      Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
  spheres.push_back(
      Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5));
  spheres.push_back(
      Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
  spheres.push_back(
      Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
  spheres.push_back(
      Sphere(Vec3f(-5.5, 0, -15), 3, Vec3f(0.90, 0.90, 0.90), 1, 0.0));
  // light
  spheres.push_back(Sphere(Vec3f(0.0, 20, -30), 3, Vec3f(0.00, 0.00, 0.00), 0,
                           0.0, Vec3f(3)));
  render(spheres);

  return 0;
}
