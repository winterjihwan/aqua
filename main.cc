#include <GLUT/GLUT.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <vector>
using namespace Eigen;
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define VIEW_WIDTH (1.5f * WINDOW_WIDTH)
#define VIEW_HEIGHT (1.5f * WINDOW_HEIGHT)

void Render(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  glLoadIdentity();
  glOrtho(0, VIEW_WIDTH, 0, VIEW_HEIGHT, 0, 1);

  glutSwapBuffers();
}

void InitGL(void) {
  glClearColor(0.9f, 0.9f, 0.9f, 1.f);
  glEnable(GL_POINT_SMOOTH);
  glMatrixMode(GL_PROJECTION);
}

int main(int argc, char **argv) {
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInit(&argc, argv);
  glutCreateWindow("Aqua");
  glutDisplayFunc(Render);

  InitGL();

  glutMainLoop();
}
