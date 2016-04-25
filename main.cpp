#include <ann_dataset.h>
#include <GL/freeglut.h>

#include "renderer.h"

size_t WindowsWidth = 800;
size_t WindowsHeight = 800;

Renderer2D renderer(WindowsWidth, WindowsHeight);

void display() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.render();
	glutSwapBuffers();
}

void init_display() {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.5);
}

void reshape(int w, int h) {
	WindowsWidth = w;
	WindowsHeight = h;
	renderer.reshape(w, h);
	glViewport(0, 0, w, h);
	init_display();
	display();
}

void mouse_wheel(int wheel, int direction, int x, int y) {
	int y1 = WindowsHeight - y;
	renderer.mouse_button(direction > 0 ? 3 : 4, GLUT_DOWN, x, y1);
}

void mouse_button(int button, int state, int x, int y) {
	int y1 = WindowsHeight - y;
	renderer.mouse_button(button, state, x, y1);
}

void mouse_active_motion(int x, int y) {
	int y1 = WindowsHeight - y;
	renderer.mouse_active_motion(x, y1);
}

void mouse_passive_motion(int x, int y) {
	int y1 = WindowsHeight - y;
	renderer.mouse_passive_motion(x, y1);
}

void normal_keys(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		break;
	case 27:
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

void init_glut_window(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WindowsWidth, WindowsHeight);
	glutCreateWindow("ANN MNIST");

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutKeyboardFunc(normal_keys);
	//glutSpecialFunc(special_keys);
	glutMouseFunc(mouse_button);
	glutMotionFunc(mouse_active_motion);
	glutPassiveMotionFunc(mouse_passive_motion);
	glutMouseWheelFunc(mouse_wheel);
	glutReshapeFunc(reshape);

	glutMainLoop();
}

int main(int argc, char *argv[]) {
  init_glut_window(argc, argv);
  printf("Testing\n");
  ann::MNISTDataset dataset("./Data/MNIST/train-images.idx3-ubyte", "./Data/MNIST/train-labels.idx1-ubyte");
  printf("Testing Ended\n");
  getchar();
  return 0;
}

