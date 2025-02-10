#include <GLUT/GLUT.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <eigen3/Eigen/Dense>
#include <vector>
using namespace Eigen;
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEW_WIDTH (1.5f * WINDOW_WIDTH)
#define VIEW_HEIGHT (1.5f * WINDOW_HEIGHT)

#define PARTICLE_SIZE 80.f
#define GRAVITY 0.02f
#define DT 2.f
#define COLLISION_DAMPLING 0.8f

// x: position
// v: velocity
struct Particle {
  Particle(float _x, float _y) : x(_x, _y), v(0.f, 0.f) {}
  Vector2d x, v;
};

static vector<Particle> particles;

void aqua_render(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  glLoadIdentity();
  glOrtho(0, VIEW_WIDTH, 0, VIEW_HEIGHT, 0, 1);

  glColor4f(0.2f, 0.6f, 1.f, 1);
  glBegin(GL_POINTS);
  for (auto &p : particles) {
    glVertex2f(p.x(0), p.x(1));
  }
  glEnd();

  glutSwapBuffers();
}

void aqua_resolve_collisions(Particle &p) {
  if (p.x(0) - PARTICLE_SIZE / 2 <= 0) {
    p.v(0) *= -1 * COLLISION_DAMPLING;
  } else if (p.x(0) + PARTICLE_SIZE / 2 >= VIEW_WIDTH) {
    p.v(0) *= -1 * COLLISION_DAMPLING;
  } else if (p.x(1) - PARTICLE_SIZE / 2 <= 0) {
    p.v(1) *= -1 * COLLISION_DAMPLING;
  } else if (p.x(1) + PARTICLE_SIZE / 2 >= VIEW_HEIGHT) {
    p.v(1) *= -1 * COLLISION_DAMPLING;
  }
}

void aqua_update(void) {
  (void)0;
  for (auto &p : particles) {
    p.v += Vector2d(0.f, -1.f) * GRAVITY * DT;
    p.x += p.v * DT;
    aqua_resolve_collisions(p);
  }
  glutPostRedisplay();
}

void aqua_init(void) {
  (void)0;
  particles.push_back(Particle(VIEW_WIDTH / 2, VIEW_HEIGHT / 2));
}

void aqua_gl_init(void) {
  glClearColor(0.9f, 0.9f, 0.9f, 1.f);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(PARTICLE_SIZE);
  glMatrixMode(GL_PROJECTION);
}

int main(int argc, char **argv) {
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInit(&argc, argv);
  glutCreateWindow("Aqua");
  glutDisplayFunc(aqua_render);
  glutIdleFunc(aqua_update);

  aqua_init();
  aqua_gl_init();

  glutMainLoop();
}
