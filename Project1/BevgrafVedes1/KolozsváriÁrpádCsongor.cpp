#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>

#include <vector>
#include <array>

GLsizei winWidth = 800, winHeight = 600;

vec2 points[11] = { { 56, 345 }, { 190, 250 }, { 286, 337 }, 
					{ 115, 451 },
				    { 326, 409 }, { 540, 455 }, { 647, 358 }, 
					{ 727, 285 }, { 764, 106 }, { 515, 60 }, { 466, 182 } };

GLint dragged = -1;

constexpr GLint numCurvePoints = 100;
std::vector<vec2> hermiteCurve(numCurvePoints);
std::vector<vec2> bernsteinBezierCurve(numCurvePoints);
std::vector<vec2> deCasteljauBezierCurve(numCurvePoints);
vec2 b[5][5];

float t0 = -2.0;
float t1 = 0.5;
float t2 = 1.5;
float u = 0.5;

vec2 e0 = (points[3] - points[0]);
vec2 e2;


void reCalculate()
{

	// Hermite

	hermiteCurve.clear();
	hermiteCurve.resize(numCurvePoints);

	float t = t0;
	float inc = (t2 - t0) / numCurvePoints;

	mat4 M = inverse(mat4{  { pow(t0, 3), pow(t1, 3), pow(t2, 3), 3 * pow(t0, 2) },
							{ pow(t0, 2), pow(t1, 2), pow(t2, 2), 2 * t0 },
							{ t0,		 t1,		 t2,		 1 },
							{ 1,			 1,			 1,			 0 } });

	e0 = (points[3] - points[0]);

	for (auto& it : hermiteCurve)
	{
		it = points[0] * (M[0][0] * pow(t, 3) + M[0][1] * pow(t, 2) + M[0][2] * t + M[0][3]) +
			 points[1] * (M[1][0] * pow(t, 3) + M[1][1] * pow(t, 2) + M[1][2] * t + M[1][3]) +
			 points[2] * (M[2][0] * pow(t, 3) + M[2][1] * pow(t, 2) + M[2][2] * t + M[2][3]) +
			 e0        * (M[3][0] * pow(t, 3) + M[3][1] * pow(t, 2) + M[3][2] * t + M[3][3]);
		t += inc;
	}

	e2 = points[0] * (M[0][0] * 3 * pow(t2, 2) + M[0][1] * 2 * t2 + M[0][2] + 0) +
		 points[1] * (M[1][0] * 3 * pow(t2, 2) + M[1][1] * 2 * t2 + M[1][2] + 0) +
		 points[2] * (M[2][0] * 3 * pow(t2, 2) + M[2][1] * 2 * t2 + M[2][2] + 0) +
		 e0        * (M[3][0] * 3 * pow(t2, 2) + M[3][1] * 2 * t2 + M[3][2] + 0);



	// Bernstein

	bernsteinBezierCurve.clear();
	bernsteinBezierCurve.resize(numCurvePoints);

	points[4] = points[2] + e2 / 3;

	t = 0.0;
	inc = 1.0 / numCurvePoints;

	for (auto& it : bernsteinBezierCurve) {
		it = points[2] * pow(1.0 - t, 3) +
			 points[4] * 3 * pow(1.0 - t, 2)*t +
			 points[5] * 3 * (1.0 - t)*pow(t, 2) +
			 points[6] * pow(t, 3);
		t += inc;
	}



	// de Casteljau

	deCasteljauBezierCurve.clear();
	deCasteljauBezierCurve.resize(numCurvePoints);

	points[7] = points[6] + (3.0 / 4.0)*(points[6] - points[5]);

	b[0][0] = points[6];
	b[0][1] = points[7];
	b[0][2] = points[8];
	b[0][3] = points[9];
	b[0][4] = points[10];

	t = 0.0;

	for (auto& it : deCasteljauBezierCurve) {
		for (int r = 1; r < 5; r++) {
			for (int i = 0; i < 5 - r; i++) {
				b[r][i] = (1.0 - t)*b[r - 1][i] + t*b[r - 1][i + 1];
			}
		}
		it = b[4][0];
		t += inc;
	}

	for (int r = 1; r < 5; r++) {
		for (int i = 0; i < 5 - r; i++) {
			b[r][i] = (1.0 - u)*b[r - 1][i] + u*b[r - 1][i + 1];
		}
	}
}

void init()
{
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0);
	reCalculate();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(1.5);

	//Hermite curve
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	{
		for (auto it : hermiteCurve) {
			glVertex2f(it.x, it.y);
		}

		glVertex2f(points[2].x, points[2].y);
	}
	glEnd();

	//Bernstein Bezier curve
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2f(points[2].x, points[2].y);

		for (auto it : bernsteinBezierCurve) {
			glVertex2f(it.x, it.y);
		}
		glVertex2f(points[6].x, points[6].y);
	}
	glEnd();

	//de Casteljau Bezier curve
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2f(points[6].x, points[6].y);

		for (auto it : deCasteljauBezierCurve) {
			glVertex2f(it.x, it.y);
		}
	}
	glEnd();

	glLineWidth(1.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	{
		glVertex2f(points[0].x, points[0].y);
		glVertex2f(points[3].x, points[3].y);

		glVertex2f(points[2].x, points[2].y);
		glVertex2f(points[2].x + e2.x, points[2].y + e2.y);

		glVertex2f(points[4].x, points[4].y);
		glVertex2f(points[5].x, points[5].y);

		glVertex2f(points[5].x, points[5].y);
		glVertex2f(points[6].x, points[6].y);

		glVertex2f(points[6].x, points[6].y);
		glVertex2f(points[7].x, points[7].y);

		glVertex2f(points[7].x, points[7].y);
		glVertex2f(points[8].x, points[8].y);

		glVertex2f(points[8].x, points[8].y);
		glVertex2f(points[9].x, points[9].y);

		glVertex2f(points[9].x, points[9].y);
		glVertex2f(points[10].x, points[10].y);
	}
	glEnd();

	glColor3f(0.58, 0.16, 0.0);
	glBegin(GL_LINES);
	{
		glVertex2f(b[1][0].x, b[1][0].y);
		glVertex2f(b[1][1].x, b[1][1].y);

		glVertex2f(b[1][1].x, b[1][1].y);
		glVertex2f(b[1][2].x, b[1][2].y);

		glVertex2f(b[1][2].x, b[1][2].y);
		glVertex2f(b[1][3].x, b[1][3].y);


		glVertex2f(b[2][0].x, b[2][0].y);
		glVertex2f(b[2][1].x, b[2][1].y);

		glVertex2f(b[2][1].x, b[2][1].y);
		glVertex2f(b[2][2].x, b[2][2].y);


		glVertex2f(b[3][0].x, b[3][0].y);
		glVertex2f(b[3][1].x, b[3][1].y);
	}
	glEnd();

	glPointSize(10.0);
	glColor3f(1.0, 0.0, 1.0);
	glBegin(GL_POINTS); 
	{
		for (int i = 0; i < 11; i++) {
			glVertex2f(points[i].x, points[i].y);
		}
	}
	glEnd();

	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_POINTS);
	{
		glVertex2f(points[2].x + e2.x, points[2].y + e2.y);
		glVertex2f(points[4].x, points[4].y);
		glVertex2f(points[7].x, points[7].y);
	}
	glEnd();

	glPointSize(6.0);
	glColor3f(0.94, 0.27, 0.0);
	glBegin(GL_POINTS);
	{
		glVertex2f(b[1][0].x, b[1][0].y);
		glVertex2f(b[1][1].x, b[1][1].y);
		glVertex2f(b[1][2].x, b[1][2].y);
		glVertex2f(b[1][3].x, b[1][3].y);

		glVertex2f(b[2][0].x, b[2][0].y);
		glVertex2f(b[2][1].x, b[2][1].y);
		glVertex2f(b[2][2].x, b[2][2].y);

		glVertex2f(b[3][0].x, b[3][0].y);
		glVertex2f(b[3][1].x, b[3][1].y);

		glVertex2f(b[4][0].x, b[4][0].y);
	}
	glEnd();

	glutSwapBuffers();
}

GLint getActivePoint1(vec2 p[], GLint size, GLint sens, GLint x, GLint y)
{
	GLint i, s = sens * sens;
	vec2 P = { (float)x, (float)y };

	for (i = 0; i < size; i++)
		if (dist2(p[i], P) < s)
			return i;
	return -1;
}

GLint getActivePoint2(vec2 *p, GLint size, GLint sens, GLint x, GLint y)
{
	GLint i;
	for (i = 0; i < size; i++)
		if (fabs((*(p + i)).x - x) < sens && fabs((*(p + i)).y - y) < sens)
			return i;
	return -1;
}

void processMouse(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
	GLint i;
	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN)
		if ((i = getActivePoint1(points, 11, 8, xMouse, winHeight - yMouse)) != -1)
			dragged = i;
	if (button == GLUT_LEFT_BUTTON && action == GLUT_UP)
		dragged = -1;
}

void processMouseActiveMotion(GLint xMouse, GLint yMouse)
{
	GLint i;
	if (dragged >= 0) {
		points[dragged].x = xMouse;
		points[dragged].y = winHeight - yMouse;
		reCalculate();
		glutPostRedisplay();
	}
}

void processSpecialKeys(int key, int x, int y) {

	switch (key) 
	{
		case GLUT_KEY_UP:
			u += 1.0 / numCurvePoints;
			if (u > 1.0) u = 1.0;
			reCalculate();
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN:
			u -= 1.0 / numCurvePoints;
			if (u < 0.0) u = 0.0;
			reCalculate();
			glutPostRedisplay();
			break;
		default: break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Drag & Drop");
	init();
	glutDisplayFunc(display);
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseActiveMotion); 
	glutSpecialFunc(processSpecialKeys);
	glutMainLoop();
	return 0;
}


