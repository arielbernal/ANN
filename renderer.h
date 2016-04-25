#ifndef RENDERER_H
#define RENDERER_H

#include "glprimitives.h"

class Renderer2D {
public:
	Renderer2D(float WW, float WH)
		: WW(WW), WH(WH) {
		x = 0; y = 0; w = WW; h = WH;
		mouse_vx = 0;
		mouse_vy = 0;
		mouse_vz = 100;
	}

	virtual void renderer() {}

	void render() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
		float k = h / w;
		glOrtho(-mouse_vz, mouse_vz, -mouse_vz, mouse_vz, -mouse_vz*k, mouse_vz * k);
		glTranslatef(mouse_vx, 0, mouse_vy);
		glRotatef(180, 0, 1, 0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPushMatrix();
		glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
		draw_axes_positive(50, 50, 50);
		renderer();
		glPopMatrix();
	}

	void mouse_button(int button, int status, int xm, int ym) {
		if (xm < x || xm > x + w || ym < y || ym > y + h) return;
		left_button_status = GLUT_UP;
		right_button_status = GLUT_UP;
		if ((button == 3) || (button == 4)) {
			if (status == GLUT_DOWN) {
				mouse_vz += button == 3 ? 3 : -3;
				if (mouse_vz < 10) mouse_vz = 10;
				if (mouse_vz > 200) mouse_vz = 200;
			}
		}
		else {
			if (button == GLUT_LEFT_BUTTON) {
				if (status == GLUT_DOWN) {
					left_button_status = GLUT_DOWN;
					left_button_down_x = xm;
					left_button_down_y = ym;
				}
			}
		}
	}
	void mouse_active_motion(int xm, int ym) {
		if (xm < x || xm > x + w || ym < y || ym > y + h) return;
		if (left_button_status == GLUT_DOWN) {
			mouse_dx = (xm - left_button_down_x);
			mouse_dy = -(ym - left_button_down_y);
			mouse_vx += mouse_dx / WW * 2 * mouse_vz;
			mouse_vy += mouse_dy / WH * 2 * mouse_vz;
		}

		left_button_down_x = xm;
		left_button_down_y = ym;
	}

	void mouse_passive_motion(int xm, int ym) {
		if (xm < x || xm > x + w || ym < y || ym > y + h) return;
		left_button_down_y = ym;
		left_button_down_x = xm;
	}

	void reshape(int width, int height) {
		WW = width; 
		WH = height;
		w = width;
		h = height;
	}
private:
	float x, y, w, h;
	
	int left_button_status;
	int right_button_status;
	int left_button_down_x;
	int left_button_down_y;
	float mouse_dx;
	float mouse_dy;
	float mouse_vx;
	float mouse_vy;
	float mouse_vz;
	float WW, WH;
};


#endif // RENDERER_H