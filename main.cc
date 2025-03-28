#include <GLUT/GLUT.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <eigen3/Eigen/Dense>
#include <vector>

using namespace Eigen;
using namespace std;

#define FPS 60
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEW_WIDTH (1.5f * WINDOW_WIDTH)
#define VIEW_HEIGHT (1.5f * WINDOW_HEIGHT)
#define BOUNDS_SIZE (Vector2d(VIEW_WIDTH, VIEW_HEIGHT))

#define NUM_PARTICLES 30
#define PARTICLE_SIZE 8.f
#define PARTICLE_SPACING 10.f
#define GRAVITY 0.005f
#define DT 2.f
#define COLLISION_DAMPLING 0.95f
#define SMOOTHING_RADIUS 160.f
#define MASS 1.f

#define TARGET_DENSITY 0.1f
static float PRESSURE_MULTIPLIER = 0.03f;

static bool PAUSE = false;

// x: position
// v: velocity
struct Particle {
  Particle(float _x, float _y) : x(_x, _y), v(0.f, 0.f) {}
  Vector2d x, v;
};

static vector<Vector2d> positions;
static vector<Vector2d> velocities(NUM_PARTICLES);
static vector<Vector2d> predicted_positions(NUM_PARTICLES);
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
  const static float H = 24.f;
  const static float EPS = H;
  for (float y = EPS; y < VIEW_HEIGHT - EPS * 2.f; y += H) {
    for (float x = VIEW_WIDTH / 4; x <= VIEW_WIDTH / 2; x += H) {
      if (positions.size() < NUM_PARTICLES) {
        float jitter =
            static_cast<float>(arc4random()) / static_cast<float>(RAND_MAX);
        positions.push_back(Vector2d(x + jitter, y));
      } else {
      }
    }
  }
}

float aqua_smoothing_kernel(float dst, float radius) {
  if (dst >= radius)
    return 0;

  float volume = (M_PI * pow(radius, 4)) / 6;
  return (radius - dst) * (radius - dst) / volume;
}

float aqua_smoothing_kernel_derivative(float dst, float radius) {
  if (dst >= radius)
    return 0;

  float scale = 12 / (pow(radius, 4) * M_PI);
  return (dst - radius) * scale;
}

float aqua_convert_density_to_pressure(float density) {
  float density_error = density - TARGET_DENSITY;
  float pressure = density_error * PRESSURE_MULTIPLIER;
  return pressure;
}

float aqua_calculate_density(Vector2d sample_point) {
  float density = 0;

  for (auto &position : positions) {
    float dst = (position - sample_point).norm();
    float influence = aqua_smoothing_kernel(dst, SMOOTHING_RADIUS);
    density += MASS * influence;
  }

  return density;
}

Vector2d aqua_get_random_dir() {
  float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
  return Vector2d(cos(angle), sin(angle));
}

float aqua_calculate_shared_pressure(float densityA, float densityB) {
  float pressureA = aqua_convert_density_to_pressure(densityA);
  float pressureB = aqua_convert_density_to_pressure(densityB);
  return (pressureA + pressureB) / 2;
}

Vector2d aqua_calculate_pressure_force(int particle_index) {
  Vector2d pressure_force = Vector2d(0.f, 0.f);

  for (int i = 0; i < NUM_PARTICLES; i++) {
    if (particle_index == i)
      continue;
    Vector2d offset = positions[i] - positions[particle_index];
    float dst = offset.norm();
    Vector2d dir = dst == 0 ? Vector2d(0.f, 0.f) : offset / dst;
    float slope = aqua_smoothing_kernel_derivative(dst, SMOOTHING_RADIUS);
    float density = densities[i];
    float shared_pressure =
        aqua_calculate_shared_pressure(density, densities[particle_index]);
    pressure_force += shared_pressure * dir * slope * MASS / density;
  }

  return -pressure_force;
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
    PRESSURE_MULTIPLIER *= 10;
    break;
  case 'b':
    PRESSURE_MULTIPLIER /= 10;
    break;
  }
}

void aqua_update(int value) {
  if (!PAUSE) {
    for (int i = 0; i < positions.size(); i++) {
      velocities[i] += Vector2d(0.f, -1.f) * GRAVITY * DT;
      predicted_positions[i] = positions[i] + velocities[i] * 1 / 120.f;
    }

    for (int i = 0; i < positions.size(); i++) {
      densities[i] = aqua_calculate_density(predicted_positions[i]);
    }

    for (int i = 0; i < positions.size(); i++) {
      Vector2d pressure_force = aqua_calculate_pressure_force(i);
      Vector2d pressure_acceleration = pressure_force / densities[i];
      velocities[i] += pressure_acceleration * DT;
    }

    for (int i = 0; i < positions.size(); i++) {
      positions[i] += velocities[i] * DT;
      aqua_resolve_collisions(positions[i], velocities[i]);
    }
  }
  glutPostRedisplay();
  glutTimerFunc(1000 / FPS, aqua_update, 0);
}

int main(int argc, char **argv) {
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInit(&argc, argv);
  glutCreateWindow("Aqua");
  glutDisplayFunc(aqua_render);
  glutTimerFunc(1000 / FPS, aqua_update, 0);
  glutKeyboardFunc(aqua_keyboard);
  aqua_init();
  aqua_gl_init();

  glutMainLoop();
}
