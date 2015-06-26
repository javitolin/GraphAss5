#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "Objects.h"
#include "GL/gl.h"
#include "GL/glut.h"
#include "Vector3f.h"

using namespace std;

#define maxSize 65535
enum Mode {
	Camera, Global, Picking
};
#define BUFSIZE 512
#define screenWidth 512
#define screenHeight 512

GLuint selectBuf[BUFSIZE];
GLfloat vertices[maxSize * 3];
GLfloat normals[maxSize * 2 * 3];
GLuint faces[maxSize * 4 * 3];
GLuint faceElements[maxSize * 2];
int vIndex, vnIndex, fIndex, eIndex;
FILE *f;
float fov;
vector<Objects*> objectsVect;
Objects* currentObject;
Mode mode;
int mouseX, mouseY;
bool leftMouseDown, rightMouseDown, middleMouseDown;
float xRotationUnits, yRotationUnits, translateForwardUnits,
		translateUpwardUnits, translateRightUnits, scaleUnits;
float xRotationAngle, yRotationAngle;
float xCameraAngle, yCameraAngle;
float translateForwardCamera, translateForwardScene, translateRightCamera,
		translateRightScene, translateUpwardCamera, translateUpwardScene;
int objectsCounter;
vector<int> hitObjects;
bool leftButtonPressed;
bool trans = false;
bool rotate = false;
bool scale = false;
bool changeFOV = false;
Vector3f camera = Vector3f(0, 0, -100);

void specialKey(int, int, int);
void readFromFile(void) {
	char c;
	f = fopen("doll.obj", "r");
	int b = 0;
	vIndex = 0;
	vnIndex = 0;
	fIndex = 0;
	eIndex = 0;
	int line = 0;
	objectsCounter = 0;
	c = fgetc(f);

	while (c != EOF && vIndex < maxSize * 3 && vnIndex < maxSize * 2 * 3
			&& fIndex < maxSize * 4 * 3) {
		line++;
		switch (c) {
		case 'o':
		case 'g':
			if (objectsCounter > 0) {
				objectsVect.push_back(currentObject);
			}
			currentObject = new Objects(objectsCounter++);
			while (c != '\n' && c != EOF)
				c = fgetc(f);
			break;
		case 'v':
			c = fgetc(f);
			if (c == 'n') {
				fscanf(f, " %f %f %f\n", &normals[vnIndex],
						&normals[vnIndex + 1], &normals[vnIndex + 2]);
				vnIndex += 3;
			} else {
				fscanf(f, "%f %f %f\n", &vertices[vIndex],
						&vertices[vIndex + 1], &vertices[vIndex + 2]);
				vIndex += 3;
			}
			break;
		case 'f':
			c = fgetc(f);
			b = 2;
			while (c != '\n' && b > 0) {
				b = fscanf(f, "%d//%d", &faces[fIndex], &faces[fIndex + 1]);
				if (b == 2) {
					fIndex += 2;
					c = fgetc(f);
					faceElements[eIndex]++;
				}
			}
			currentObject->addFace(eIndex);
			eIndex++;
			break;
		default:
			while (c != '\n' && c != EOF) //Ignore Comments
				c = fgetc(f);
			break;
		}
		if (c != EOF)
			c = fgetc(f);
	}
	objectsVect.push_back(currentObject);
	fclose(f);
	printf("indices %d %d %d %d %d\n", vIndex, vnIndex, fIndex, eIndex, line);
}

void calculateCOM(void) {
	int i = 0, x, y, z, num_of_vert;
	vector<int> faces_vect;
	vector<Objects*>::iterator it;
	for (it = objectsVect.begin(); it != objectsVect.end(); ++it) {
		x = y = z = num_of_vert = 0;
		faces_vect = (*it)->getFaces();
		vector<int>::iterator it2;
		for (it2 = faces_vect.begin(); it2 != faces_vect.end(); ++it2) {
			for (unsigned int j = 0; j < faceElements[*it2] * 2; j += 2) {
				x += vertices[(faces[i + j] - 1) * 3];
				y += vertices[(faces[i + j] - 1) * 3 + 1];
				z += vertices[(faces[i + j] - 1) * 3 + 2];
				num_of_vert++;
			}
			i += faceElements[*it2] * 2;
		}
		x = x / num_of_vert;
		y = y / num_of_vert;
		z = z / num_of_vert;
		(*it)->setCenter(x, y, z);
	}
}

void init(void) {
	leftMouseDown = false;
	rightMouseDown = false;
	middleMouseDown = false;
	fov = 60.0;
	mode = Camera;
	mouseX = mouseY = numeric_limits<int>::max();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, screenWidth / screenHeight, 2, 200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);

	//--------------------------------------------LIGHT----------------------------------------//
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_diffuse[] = { 0.0, 0.5, 0.5, 1.0 };
	GLfloat light_specular[] = { 0.0, 0.0, 0.5, 1.0 };
	GLfloat light_position[] = { -1.0, 1.0, -1.0, 0.0 };

	GLfloat object_ambient[] = { 0.3, 0.4, 0.5, 1.0 };
	GLfloat object_diffuse[] = { 0.0, 0.6, 0.7, 1.0 };
	GLfloat object_specular[] = { 0.0, 0.0, 0.8, 1.0 };
	GLfloat object_shine[] = { 5 };
	GLfloat mat_shin[] = { 5.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SHININESS, mat_shin);

	glEnable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT, GL_AMBIENT, object_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, object_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, object_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, object_shine);
	//--------------------------------------------LIGHT----------------------------------------//
	readFromFile();
	calculateCOM();
}

void draw(GLenum drawing_mode) {
	//Draw the axis
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(100.0, 0.0, 0.0);

	glColor3f(0, 0, 1);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 100.0, 0.0);

	glColor3f(0, 1, 0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 100.0);
	glEnd();
	glColor4f(1.0, 1.0, 1.0, 1.0);

	if (changeFOV) {
		changeFOV = false;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, screenWidth / screenHeight, 2, 200);
		glTranslatef(camera.x, camera.y, camera.z);
		glMatrixMode(GL_MODELVIEW);
	}

	int i = 0;
	vector<int> faces_vect;
	vector<Objects*>::iterator it;
	for (it = objectsVect.begin(); it != objectsVect.end(); ++it) {
		faces_vect = (*it)->getFaces();
		if ((*it)->isSelected()) {
			glColor3f(1, 0, 0);
		}
		if (drawing_mode == GL_SELECT)
			glLoadName((*it)->getId());

		glPushMatrix();
		//Save current matrix & Load Identity
		float mview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mview);
		glLoadIdentity();
		//Translate the object
		glTranslatef((*it)->getTranslationX(), (*it)->getTranslationY(),
				(*it)->getDepth());
		//Multiply by saved matrix
		glMultMatrixf(mview);
		//Rotate the object
		vector<float> center_vect = (*it)->getCenter();
		glTranslatef(center_vect.at(0), center_vect.at(1), center_vect.at(2));
		glRotatef((*it)->getAngleX(), 1.0, 0.0, 0.0);
		glRotatef((*it)->getAngleY(), 0.0, 1.0, 0.0);
		glTranslatef(-center_vect.at(0), -center_vect.at(1),
				-center_vect.at(2)); //Translate to origin

		vector<int>::iterator it2;
		for (it2 = faces_vect.begin(); it2 != faces_vect.end(); ++it2) {
			glBegin(GL_POLYGON);
			for (unsigned int j = 0; j < faceElements[*it2] * 2; j += 2) {
				glNormal3f(normals[(faces[i + j + 1] - 1) * 3],
						normals[(faces[i + j + 1] - 1) * 3 + 1],
						normals[(faces[i + j + 1] - 1) * 3 + 2]);
				glVertex3f(vertices[(faces[i + j] - 1) * 3],
						vertices[(faces[i + j] - 1) * 3 + 1],
						vertices[(faces[i + j] - 1) * 3 + 2]);
			}
			glEnd();
			i += faceElements[*it2] * 2;
		}
		glPopMatrix();
		if ((*it)->isSelected()) {
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
	}
}

float scale_x, scale_y, scale_z;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glScalef(scale_x, scale_y, scale_z);
	glTranslatef(translateRightScene, translateUpwardScene, 0);
	glRotatef(xRotationAngle, 0.0, 1.0, 0.0);
	glRotatef(yRotationAngle, 1.0, 0.0, 0.0);
	draw(GL_RENDER);
	glPopMatrix();
	glFlush();
}

void specialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_F2:
		fov -= 0.05;
		changeFOV = true;
		break;
	case GLUT_KEY_F3:
		fov += 0.05;
		changeFOV = true;
		break;
	case GLUT_KEY_UP:
		scale_x = scale_x * 1.05;
		scale_y = scale_y * 1.05;
		scale_z = scale_z * 1.05;
		break;
	case GLUT_KEY_DOWN:
		scale_x = scale_x * 0.95;
		scale_y = scale_y * 0.95;
		scale_z = scale_z * 0.95;
		break;
	default:
		cout << "Not recognized" << endl;
		break;
	}
}

void processHits(int hits) {
	GLuint minZ, *ptr, names, *ptrNames;

	ptr = (GLuint*) selectBuf;
	minZ = 0xffffffff;
	for (int i = 0; i < hits; i++) {
		names = *ptr;
		ptr++;
		if (*ptr < minZ) {
			minZ = *ptr;
			ptrNames = ptr + 2;
		}
		ptr += names + 2; //Go for the next hit
	}
	if (leftMouseDown) {
		hitObjects.push_back(*ptrNames);
		(objectsVect.at(*ptrNames))->selected(true);
		printf("Object %d will be selected\n", *ptrNames);
	} else if (rightMouseDown) {
		(objectsVect.at(*ptrNames))->selected(false);
		for (unsigned int i = 0; i < hitObjects.size(); i++) {
			if (hitObjects.at(i) == *ptrNames) {
				hitObjects.erase(hitObjects.begin() + i);
				printf("Object %d will not be selected\n", *ptrNames);
				break;
			}
		}
	}
}

void startPicking(int x, int y) {
	int hits;
	GLint viewport[4];
	glSelectBuffer(BUFSIZE, selectBuf);
	glRenderMode(GL_SELECT);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(x, viewport[3] - y, 5, 5, viewport);
	gluPerspective(fov, screenWidth / screenHeight, 2.0, 200.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); //Add Global&Camera modes chaings
	glScalef(scale_x, scale_y, scale_z);
	glTranslatef(translateRightScene, translateUpwardScene, 0);
	glRotatef(xRotationAngle, 0.0, 1.0, 0.0);
	glRotatef(yRotationAngle, 1.0, 0.0, 0.0);
	draw(GL_SELECT);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();
	if ((hits = glRenderMode(GL_RENDER)) != 0) {
		processHits(hits);
	}
	glMatrixMode(GL_MODELVIEW);
}
void idle(int v) {
	string windowName = "AMAZING 3D MODELING - ";
	if(mode == Camera){
		windowName += "CAMERA";
	}
	else if(mode == Global){
		windowName += "GLOBAL";
	}
	else if(mode == Picking){
		windowName += "PICKING";
		if(trans){
			windowName += " - Translate";
		}
		else if(rotate){
			windowName += " - Rotate";
		}
		else if(scale){
			windowName += " - Scale";
		}
	}
	glutSetWindowTitle(windowName.c_str());
	glutPostRedisplay();
	glutTimerFunc(1, idle, 0);
}
void keyboard(unsigned char key, int x, int y) {

	switch (key) {
	//Esc
	case 27:
		exit(0);
		break;
		//Backspace
	case 'c':
		mode = Camera;
		scale = false;
		trans = false;
		rotate = false;
		break;

	case 'g':
		mode = Global;
		scale = false;
		trans = false;
		rotate = false;
		break;
	case 'p':
		if (mode != Picking) {
			mode = Picking;
		} else {
			mode = Camera;
		}
		break;
	case 't':
		if (mode == Picking) {
			trans = true;
			rotate = false;
			scale = false;
		}
		break;
	case 'r':
		if (mode == Picking) {
			rotate = true;
			scale = false;
			trans = false;
		}
		break;
	case 's':
		if (mode == Picking) {
			scale = true;
			trans = false;
			rotate = false;
		}
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	mouseX = x;
	mouseY = y;
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			leftMouseDown = true;
			if (mode == Picking)
				startPicking(x, y);
		} else
			leftMouseDown = false;
	} else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			rightMouseDown = true;
			if (mode == Picking) {
				startPicking(x, y);
			}
		} else
			rightMouseDown = false;
	} else {
		if (state == GLUT_DOWN) {
			middleMouseDown = true;
			if (mode == Picking)
				startPicking(x, y);
		} else
			middleMouseDown = false;
	}
}

void motion(int x, int y) {
	int difference_x = mouseX - x;
	int difference_y = mouseY - y;
	float angle_x = -1.0 * float(mouseX - x) * xRotationUnits;
	float angle_y = -1.0 * float(mouseY - y) * yRotationUnits;
	if (leftMouseDown) {
		if (mode == Camera) {
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			xCameraAngle += angle_x;
			yCameraAngle += angle_y;
			if (xCameraAngle > 360 || xCameraAngle < -360)
				xCameraAngle = 0;
			if (yCameraAngle > 360 || yCameraAngle < -360)
				yCameraAngle = 0;
			glRotatef(angle_x, 0.0, 1.0, 0.0);
			glRotatef(angle_y, 1.0, 0.0, 0.0);
			glMultMatrixf(mview);
		} else if (mode == Global) {
			xRotationAngle += angle_x;
			yRotationAngle += angle_y;
			if (xRotationAngle > 360 || xRotationAngle < -360)
				xRotationAngle = 0;
			if (yRotationAngle > 360 || yRotationAngle < -360)
				yRotationAngle = 0;
		} else {
			for (unsigned int i = 0; i < hitObjects.size(); i++) {
				if (trans) {
					objectsVect.at(hitObjects.at(i))->updateTranslation(
							-difference_x * translateRightUnits,
							difference_y * translateUpwardUnits);
				}
				if (scale) {
					objectsVect.at(hitObjects.at(i))->updateDepth(
							difference_y * translateForwardUnits);

				}
				if (rotate) {
					objectsVect.at(hitObjects.at(i))->updateAngles(angle_x,
							angle_y);
				}
			}
		}
	}
	if (rightMouseDown) {
		if (mode == Camera) {
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			translateUpwardCamera += -difference_y * translateUpwardUnits;
			translateRightCamera += difference_x * translateRightUnits;
			glLoadIdentity();
			glTranslatef(difference_x * translateRightUnits,
					-difference_y * translateUpwardUnits, 0);
			glMultMatrixf(mview);

		} else if (mode == Global) {
			translateUpwardScene += difference_y * translateUpwardUnits;
			translateRightScene += -difference_x * translateRightUnits;
		}
	}
	if (middleMouseDown) {
		if (mode == Camera) {
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			translateForwardCamera += difference_y * translateForwardUnits;
			glLoadIdentity();
			glTranslatef(0, 0, difference_y * translateForwardUnits);
			glMultMatrixf(mview);
		} else if (mode == Global) {
			scale_x += difference_y * scaleUnits;
			scale_y += difference_y * scaleUnits;
			scale_z += difference_y * scaleUnits;
			if (scale_x < 0.01) {
				scale_x = scale_y = scale_z = 0.01;
			}
		}
	}
	mouseX = x;
	mouseY = y;
}

int main(int argc, char **argv) {
	xCameraAngle = 0;
	yCameraAngle = 0;
	translateForwardScene = 0;
	translateForwardCamera = -20;
	translateRightScene = 0;
	translateRightCamera = 0;
	translateUpwardScene = 0;
	translateUpwardCamera = 0;
	xRotationUnits = 180.0 / (float) screenWidth;
	yRotationUnits = 180.0 / (float) screenHeight;
	translateForwardUnits = 5.0 / (float) screenHeight;
	translateUpwardUnits = 5.0 / (float) screenHeight;
	translateRightUnits = 5.0 / (float) screenWidth;
	scaleUnits = 1.0 / (float) screenHeight;
	scale_x = scale_y = scale_z = 1.0;
	xRotationAngle = 0;
	yRotationAngle = 0;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(150, 150);
	glutCreateWindow("");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(2, idle, 1);
	glutMainLoop();
	return 0;
}
