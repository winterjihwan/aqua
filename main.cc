#include <GLUT/GLUT.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <cmath>
#include <cstdlib>
#include <eigen3/Eigen/Dense>
#include <vector>

using namespace Eigen;
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEW_WIDTH (1.5f * WINDOW_WIDTH)
#define VIEW_HEIGHT (1.5f * WINDOW_HEIGHT)
#define BOUNDS_SIZE (Vector2d(VIEW_WIDTH, VIEW_HEIGHT))

#define PARTICLE_SIZE 8.f
#define PARTICLE_SPACING 10.f
#define GRAVITY 0.04f
#define DT 2.f
#define COLLISION_DAMPLING 0.9f
#define NUM_PARTICLES 100
#define SMOOTHING_RADIUS 160.f

#define TARGET_DENSITY 0.1f
static float PRESSURE_MULTIPLIER = 0.01f;

static bool PAUSE = false;

// x: position
// v: velocity
struct Particle {
  Particle(float _x, float _y) : x(_x, _y), v(0.f, 0.f) {}
  Vector2d x, v;
};

static vector<Vector2d> positions(NUM_PARTICLES);
static vector<Vector2d> velocities(NUM_PARTICLES);
static vector<float> densities(NUM_PARTICLES);

void aqua_render(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  glLoadIdentity();
  glOrtho(0, VIEW_WIDTH, 0, VIEW_HEIGHT, 0, 1);

  glColor4f(0.2f, 0.6f, 1.f, 1);
  glBegin(GL_POINTS);
  for (auto &p : positions) {
    glVertex2f(p(0), p(1));
  }
  glEnd();

  glutSwapBuffers();
}

void aqua_resolve_collisions(Vector2d &p, Vector2d &v) {
  Vector2d half_bounds_size =
      BOUNDS_SIZE / 2 - Vector2d(1.f, 1.f) * PARTICLE_SIZE;

  float x = p(0) - VIEW_WIDTH / 2;
  float y = p(1) - VIEW_HEIGHT / 2;

  if (abs(x) > half_bounds_size(0)) {
    x = copysign(half_bounds_size(0), x);
    v(0) *= -1 * COLLISION_DAMPLING;
  }
  if (abs(y) > half_bounds_size(1)) {
    y = copysign(half_bounds_size(1), y);
    v(1) *= -1 * COLLISION_DAMPLING;
  }

  p(0) = x + VIEW_WIDTH / 2;
  p(1) = y + VIEW_HEIGHT / 2;
}

void aqua_init(void) {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    float x = arc4random() % (int)VIEW_WIDTH;
    float y = arc4random() % (int)VIEW_HEIGHT;

    positions[i] = Vector2d(x, y);
  }
}

float aqua_smoothing_kernel(float dst, float radius) {
  float volume = M_PI * pow(radius, 8);
  float value = fmaxf(0, radius * radius - dst * dst);
  return value * value * value / volume;
}

float aqua_smoothing_kernel_derivative(float dst, float radius) {
  if (dst >= radius)
    return 0;
  float f = radius * radius - dst * dst;
  float scale = -24 / (M_PI * pow(radius, 8));
  return scale * dst * f * f;
}

float aqua_convert_density_to_pressure(float density) {
  float density_error = density - TARGET_DENSITY;
  float pressure = density_error * PRESSURE_MULTIPLIER;
  return pressure;
}

float aqua_calculate_density(Vector2d sample_point) {
  float density = 0;
  const float mass = 1;

  for (auto &position : positions) {
    float dst = (position - sample_point).norm();
    float influence = aqua_smoothing_kernel(dst, SMOOTHING_RADIUS);
    density += mass * influence;
  }

  return density;
}

void aqua_update_densities() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    densities[i] = aqua_calculate_density(positions[i]);
  }
}

Vector2d aqua_get_random_dir() {
  float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
  return Vector2d(std::cos(angle), std::sin(angle));
}

Vector2d aqua_calculate_pressure_force(int particle_index) {
  Vector2d pressure_force = Vector2d(0.f, 0.f);
  const float mass = 1.f;

  for (int i = 0; i < NUM_PARTICLES; i++) {
    if (particle_index == i)
      continue;
    Vector2d offset = positions[i] - positions[particle_index];
    float dst = offset.norm();
    Vector2d dir = dst == 0 ? aqua_get_random_dir() : offset / dst;
    float slope = aqua_smoothing_kernel_derivative(dst, SMOOTHING_RADIUS);
    float density = densities[i];
    pressure_force += -aqua_convert_density_to_pressure(density) * dir * slope *
                      mass / density;
  }

  return pressure_force;
}

void aqua_gl_init(void) {
  glClearColor(0.9f, 0.9f, 0.9f, 1.f);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(2 * PARTICLE_SIZE);
  glMatrixMode(GL_PROJECTION);
}

void aqua_keyboard(unsigned char c, int x, int y) {
  switch (c) {
  case 'n':
    break;
  case 'r':
  case 'R':
    aqua_init();
    break;
  case 'p':
    PAUSE = !PAUSE;
  case 'a':
    PRESSURE_MULTIPLIER *= 2;
  case 'b':
    PRESSURE_MULTIPLIER /= 2;
  }
}

void aqua_update(void) {
  if (PAUSE) {
    for (int i = 0; i < positions.size(); i++) {
      velocities[i] += Vector2d(0.f, 1.f) * GRAVITY * DT;
      densities[i] = aqua_calculate_density(positions[i]);
    }

    for (int i = 0; i < positions.size(); i++) {
      Vector2d pressure_force = aqua_calculate_pressure_force(i);
      Vector2d pressure_acceleration = pressure_force / densities[i];
      velocities[i] = pressure_acceleration;
    }

    for (int i = 0; i < positions.size(); i++) {
      positions[i] += velocities[i] * DT;
      aqua_resolve_collisions(positions[i], velocities[i]);
    }
  }
  glutPostRedisplay();
}

int main(int argc, char **argv) {
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInit(&argc, argv);
  glutCreateWindow("Aqua");
  glutDisplayFunc(aqua_render);
  glutIdleFunc(aqua_update);
  glutKeyboardFunc(aqua_keyboard);

  aqua_init();
  aqua_gl_init();

  glutMainLoop();
}
