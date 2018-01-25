#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>

#include <vector>
#include <array>
#include <algorithm>

GLsizei winWidth = 800, winHeight = 600;

mat4 w2v, projection, camera, torusRotate;

float cameraDistance = 10.0f, cameraHeight = 0.0f, cameraAngle = 0.0f, torusAngle = 0.0f;
vec3 centerOfPerspective{ 0.0f, 0.0f, 5.0f };

vec3 lightDirection{ 0.0f,0.0f,1.0f };
vec3 lightColor{ 1.0f,1.0f,1.0f };

vec3 cameraPos;

enum ProjectionType { MEROLEGES, CENTRALIS };

ProjectionType projectionType = CENTRALIS;

class Face
{
public:
	Face(std::vector<vec3>* model, GLuint v1, GLuint v2, GLuint v3, GLuint v4) : model(model), v1(v1), v2(v2), v3(v3), v4(v4), color(1.0f, 0.0f, 0.0f)
	{
		normal = normalize(cross(model->at(v1) - model->at(v2), model->at(v1) - model->at(v4)));
		centerPoint = (model->at(v1) + model->at(v2) + model->at(v3) + model->at(v4)) / 4.0f;
	}

	void drawFace() const {
		glColor3f(color.x, color.y, color.z);
		glBegin(GL_TRIANGLES);
		{
			glVertex2f(model->at(v1).x, model->at(v1).y);
			glVertex2f(model->at(v2).x, model->at(v2).y);
			glVertex2f(model->at(v3).x, model->at(v3).y);

			glVertex2f(model->at(v3).x, model->at(v3).y);
			glVertex2f(model->at(v4).x, model->at(v4).y);
			glVertex2f(model->at(v1).x, model->at(v1).y);
		}
		glEnd();
	}

	void drawFaceContour() const {
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_LOOP);
		{
			glVertex2f(model->at(v1).x, model->at(v1).y);
			glVertex2f(model->at(v2).x, model->at(v2).y);
			glVertex2f(model->at(v3).x, model->at(v3).y);
			glVertex2f(model->at(v4).x, model->at(v4).y);
		}
		glEnd();
	}

	void recalculateNormalsAndCenter()
	{
		normal = normalize(cross(model->at(v1) - model->at(v2), model->at(v1) - model->at(v4)));
		centerPoint = (model->at(v1) + model->at(v2) + model->at(v3) + model->at(v4)) / 4.0f;
	}

	void setColor(vec3 color)
	{
		this->color = color;
	}

	std::vector<vec3>* getModel()
	{
		return model;
	}

	const vec3& getNormal() const
	{
		return normal;
	}

	const vec3& getCenterPoint() const
	{
		return centerPoint;
	}

private:
	std::vector<vec3>* model;
	GLuint v1, v2, v3, v4;
	vec3 normal;
	vec3 centerPoint;
	vec3 color;
};

const std::vector<vec3> cubeVertices{
	{ -0.5, 0.5, 0.5 },    // 0
	{ 0.5, 0.5, 0.5 },     // 1
	{ 0.5, 0.5, -0.5 },    // 2
	{ -0.5, 0.5, -0.5 },   // 3
	{ -0.5, -0.5, 0.5 },   // 4
	{ 0.5, -0.5, 0.5 },    // 5
	{ 0.5, -0.5, -0.5 },   // 6
	{ -0.5, -0.5, -0.5 } };// 7

std::vector<vec3> torusVertices;
int torusFaces = 10;
float torusVerticalRadius = 0.5f;
float torusHorizontalRadius = 3.0f;


std::vector<Face> setupCubeFaces(std::vector<vec3>* cube)
{
	std::vector<Face> cubeFaces{
		Face(cube, 0, 4, 5, 1),
		Face(cube, 1, 5, 6, 2),
		Face(cube, 2, 6, 7, 3),
		Face(cube, 3, 7, 4, 0),
		Face(cube, 3, 0, 1, 2),
		Face(cube, 4, 7, 6, 5) };

	return cubeFaces;
}

std::vector<Face> setupTorusFaces(std::vector<vec3>* torus)
{
	std::vector<Face> torusFaces;

	for (int i = 0; i < torus->size(); i += 4)
	{
		torusFaces.push_back(Face(torus, i, i + 1, i + 2, i + 3));
	}

	return torusFaces;
}


vec3 getPointOnTorus(GLfloat u, GLfloat v)
{
	vec3 point;

	point.x = (torusHorizontalRadius + torusVerticalRadius * cos(u)) * cos(v);
	point.y = (torusHorizontalRadius + torusVerticalRadius * cos(u)) * sin(v);
	point.z = torusVerticalRadius * sin(u);

	return point;
}

void recalculateTorus()
{
	torusVertices.clear();

	float inc = (2 * pi()) / torusFaces;

	for (float u = 0.0f; u <= (2*pi()); u += inc) {
		for (float v = 0.0f; v <= ( 2*pi()); v += inc) {
			torusVertices.push_back(getPointOnTorus(u, v));
			torusVertices.push_back(getPointOnTorus(u, v - inc));
			torusVertices.push_back(getPointOnTorus(u - inc, v - inc));
			torusVertices.push_back(getPointOnTorus(u - inc, v));
		}
	}
}


void setCamera(vec3 position, vec3 target, vec3 up)
{
	vec3 camZ = normalize(position - target);
	vec3 camX = normalize(cross(up, camZ));
	vec3 camY = normalize(cross(camZ, camX));

	camera = coordinateTransform(position, camX, camY, camZ);
}

void initMatrices()
{
	vec2 windowSize = { 2, 2 };
	vec2 windowPosition = { -1, -1 };
	vec2 viewportSize = { 150, 150 };
	vec2 viewportPosition = { winWidth / 2 - viewportSize.x / 2, winHeight / 2 - viewportSize.y / 2 };

	w2v = windowToViewport3(windowPosition, windowSize, viewportPosition, viewportSize);

	projection = perspective(centerOfPerspective.z);
	projectionType = CENTRALIS;

	cameraPos = vec3(cameraDistance*cos(degToRad(cameraAngle)), cameraDistance*sin(degToRad(cameraAngle)), cameraHeight);
	setCamera(cameraPos, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
}

void init()
{
	glClearColor(0.7, 0.7, 0.7, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
	glShadeModel(GL_FLAT);
	glLineWidth(1.0);

	initMatrices();

	recalculateTorus();
}



std::vector<vec3> applyTransform(const std::vector<vec3>& model, mat4 matrix)
{
	std::vector<vec3> transformedModel;

	for (auto it : model) {
		vec4 v = matrix*ihToH(it);
		if (v.w != 0)
			transformedModel.push_back(hToIh(v));
	}

	return transformedModel;
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// CUBE

	std::vector<vec3> transformedCube = cubeVertices;
	std::vector<Face> cubeFaces = setupCubeFaces(&transformedCube);

	for (auto& it : cubeFaces) {
		GLfloat lightIntensity = (dot(it.getNormal(), normalize(lightDirection)) + 1.0f) / 2.0f;
		it.setColor(lightIntensity*lightColor);
	}

	transformedCube = applyTransform(transformedCube, camera);
	for (auto& it : cubeFaces) {
		it.recalculateNormalsAndCenter();
	}

	// TORUS

	std::vector<vec3> transformedTorus = torusVertices;
	std::vector<Face> torusFaces = setupTorusFaces(&transformedTorus);
	transformedTorus = applyTransform(transformedTorus, torusRotate);

	for (auto& it : torusFaces) {
		GLfloat lightIntensity = (dot(it.getNormal(), normalize(lightDirection)) + 1.0f) / 2.0f;
		it.setColor(lightIntensity*lightColor);
	}

	transformedTorus = applyTransform(transformedTorus, camera);
	for (auto& it : torusFaces) {
		it.recalculateNormalsAndCenter();
	}

	// DRAW

	std::vector<Face> allFaces = cubeFaces;
	allFaces.reserve(cubeFaces.size() + torusFaces.size());
	allFaces.insert(allFaces.end(), torusFaces.begin(), torusFaces.end());

	switch (projectionType) {
	case CENTRALIS:
		std::remove_if(allFaces.begin(), allFaces.end(),
			[&](const Face& face) {
			return dot(centerOfPerspective - face.getCenterPoint(), face.getNormal()) < 0.0;
		});

		std::sort(allFaces.begin(), allFaces.end(),
			[](const Face& a, const Face& b) {
			return length(centerOfPerspective - a.getCenterPoint()) > length(centerOfPerspective - b.getCenterPoint());
		});

		break;

	case MEROLEGES:
		std::remove_if(allFaces.begin(), allFaces.end(),
			[&](const Face& face) {
			return face.getNormal().z < 0.0;
		});

		std::sort(allFaces.begin(), allFaces.end(),
			[](const Face& a, const Face& b) {
			return a.getCenterPoint().z < b.getCenterPoint().z;
		});

		break;
	}

	transformedTorus = applyTransform(transformedTorus, w2v*projection);
	transformedCube = applyTransform(transformedCube, w2v*projection);

	for (auto it : allFaces) { // ez cuz pointers, lol ^
		it.drawFace();
		it.drawFaceContour();
	}

	glutSwapBuffers();
}

template<typename T>
void checkForMinimum(T& value, T minimum)
{
	if (value < minimum) value = minimum;
}

void keyboard(unsigned char key, int x, int y)
{
	bool needCameraUpdate = false;
	bool needTorusUpdate = false;
	bool needProjectionUpdate = false;

	switch (key) {
	case 27:
		exit(0);
		break;
	case 'w':
		cameraDistance -= 0.5f;
		checkForMinimum(cameraDistance, 1.0f);
		needCameraUpdate = true;
		break;
	case 'a':
		cameraAngle += 1.0f;
		needCameraUpdate = true;
		break;
	case 's':
		cameraDistance += 0.5f;
		needCameraUpdate = true;
		break;
	case 'd':
		cameraAngle -= 1.0f;
		needCameraUpdate = true;
		break;
	case 'q':
		cameraHeight -= 0.3f;
		needCameraUpdate = true;
		break;
	case 'e':
		cameraHeight += 0.3f;
		needCameraUpdate = true;
		break;

	case 'i':
		torusFaces += 1;
		needTorusUpdate = true;
		break;
	case 'k':
		torusFaces -= 1;
		checkForMinimum(torusFaces, 3);
		needTorusUpdate = true;
		break;
	case 'u':
		torusHorizontalRadius += 0.1f;
		needTorusUpdate = true;
		break;
	case 'j':
		torusHorizontalRadius -= 0.1f;
		checkForMinimum(torusHorizontalRadius, 1.0f);
		needTorusUpdate = true;
		break;
	case 'o':
		torusVerticalRadius += 0.1f;
		needTorusUpdate = true;
		break;
	case 'l':
		torusVerticalRadius -= 0.1f;
		checkForMinimum(torusVerticalRadius, 0.1f);
		needTorusUpdate = true;
		break;

	case 'n':
		centerOfPerspective.z -= 0.2f;
		checkForMinimum(centerOfPerspective.z, 1.0f);
		needProjectionUpdate = true;
		break;

	case 'm':
		centerOfPerspective.z += 0.2f;
		needProjectionUpdate = true;
		break;

	case 'p':
		switch (projectionType) {
		case MEROLEGES:
			projectionType = CENTRALIS;
			break;
		case CENTRALIS:
			projectionType = MEROLEGES;
			break;
		}
		needProjectionUpdate = true;
		break;

	default: break;
	}

	if (needCameraUpdate) {
		cameraPos = vec3(cameraDistance*cos(degToRad(cameraAngle)), cameraDistance*sin(degToRad(cameraAngle)), cameraHeight);
		setCamera(cameraPos, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
	}

	if (needTorusUpdate) {
		recalculateTorus();
	}

	if (needProjectionUpdate) {
		switch (projectionType) {
		case MEROLEGES:
			projection = ortho();
			break;
		case CENTRALIS:
			projection = perspective(centerOfPerspective.z);
			break;
		}
	}

	glutPostRedisplay();
}

void update(int value)
{
	torusAngle += pi() / 200;
	torusRotate = rotateZ(torusAngle);

	glutPostRedisplay();

	glutTimerFunc(10, update, 0);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(500, 300);
	glutCreateWindow("Cube in a torus");

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(10, update, 0);

	glutMainLoop();
	return 0;
}