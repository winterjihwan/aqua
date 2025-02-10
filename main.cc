#include <GLUT/GLUT.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <vector>
using namespace Eigen;
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEW_WIDTH (1.5f * WINDOW_WIDTH)
#define VIEW_HEIGHT (1.5f * WINDOW_HEIGHT)
#define BOUNDS_SIZE (Vector2d(VIEW_WIDTH, VIEW_HEIGHT))

#define PARTICLE_SIZE 40.f
#define PARTICLE_SPACING 10.f
#define GRAVITY 0.04f
#define DT 2.f
#define COLLISION_DAMPLING 0.9f
#define NUM_PARTICLES 16
#define SMOOTHING_RADIUS 160.f

static bool PAUSE = false;

// x: position
// v: velocity
struct Particle {
  Particle(float _x, float _y) : x(_x, _y), v(0.f, 0.f) {}
  Vector2d x, v;
};

static vector<Vector2d> positions(NUM_PARTICLES);
static vector<Vector2d> velocities(NUM_PARTICLES);

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
    x = half_bounds_size(0) * -1;
    v(0) *= -1 * COLLISION_DAMPLING;
  }
  if (abs(y) > half_bounds_size(1)) {
    y = half_bounds_size(1) * -1;
    v(1) *= -1 * COLLISION_DAMPLING;
  }

  p(0) = x + VIEW_WIDTH / 2;
  p(1) = y + VIEW_HEIGHT / 2;
}

void aqua_update(void) {
  (void)0;
  if (!PAUSE) {
    for (int i = 0; i < positions.size(); i++) {
      velocities[i] += Vector2d(0.f, -1.f) * GRAVITY * DT;
      positions[i] += velocities[i] * DT;
      aqua_resolve_collisions(positions[i], velocities[i]);
    }
  }
  glutPostRedisplay();
}

void aqua_init(void) {
  int particles_per_row = (int)sqrt(NUM_PARTICLES);
  int particles_per_col = (NUM_PARTICLES - 1) / particles_per_row + 1;
  float spacing = PARTICLE_SIZE * 2 + PARTICLE_SPACING;

  for (int i = 0; i < NUM_PARTICLES; i++) {
    float x =
        (i % particles_per_row - particles_per_row / 2.f + 0.5f) * spacing;
    float y = ((float)i / particles_per_row - particles_per_col / 2.f + 0.5f) *
              spacing;
    positions[i] = Vector2d(x + VIEW_WIDTH / 2, y + VIEW_HEIGHT / 2);
  }
}

float aqua_smoothing_kernel(float radius, float dst) {
  float value = fmaxf(0, radius * radius - dst * dst);
  return value * value * value;
}

float aqua_calculate_density(Vector2d sample_point) {
  float density = 0;
  const float mass = 1;

  for (auto &position : positions) {
    float dst = (position - sample_point).norm();
    float influence = aqua_smoothing_kernel(SMOOTHING_RADIUS, dst);
    density += mass * influence;
  }

  return density;
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
  }
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
