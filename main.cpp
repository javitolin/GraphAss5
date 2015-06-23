#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "Objects.h"
#include "GL/gl.h"
#include "GL/glut.h"
using namespace std;

#define maxSize 65535
enum Mode {Camera, Global, Picking};
#define BUFSIZE 512

GLuint selectBuf[BUFSIZE];
int window_size_x, window_size_y;
GLfloat vertices[maxSize*3];
GLfloat normals[maxSize*2*3];
GLuint faces[maxSize*4*3];
GLuint faceElements[maxSize*2];
int vIndex,vnIndex,fIndex,eIndex;
FILE *f;
float fov;
vector<Objects*> objects_vect;
Objects* current_object;
Mode mode;
int mouse_x, mouse_y;
bool left_mouse_down, right_mouse_down, middle_mouse_down;
float x_rotation_units, y_rotation_units, translate_forward_units, translate_upward_units, translate_right_units, scale_units;
float x_rotation_angle, y_rotation_angle;
float x_camera_angle, y_camera_angle;
float translate_forward_camera, translate_forward_scene, translate_right_camera, translate_right_scene, translate_upward_camera, translate_upward_scene;
int objects_counter;
vector<int> hitObjects;
bool leftButtonPressed;
bool allPressed = false;

void reader(void)
{
	char c;
	f=fopen("doll.obj","r");
	int b=0;
	vIndex=0;
	vnIndex=0;
	fIndex=0;
	eIndex=0;
	int line=0;
	objects_counter = 0;
	c=fgetc(f);

	while(c!=EOF && vIndex<maxSize*3 && vnIndex<maxSize*2*3 && fIndex<maxSize*4*3)
	{
		line++;
		switch(c)
		{
			case 'o':
			case 'g':
				if(objects_counter > 0)
				{
					objects_vect.push_back(current_object);
				}
				current_object = new Objects(objects_counter++);
				while(c!='\n' && c!=EOF)
					c = fgetc(f);
			break;
			case 'v':
				c=fgetc(f);
				if(c=='n')
				{
					fscanf(f," %f %f %f\n",&normals[vnIndex],&normals[vnIndex+1],&normals[vnIndex+2]);
					vnIndex+=3;
				}
				else
				{
					fscanf(f,"%f %f %f\n",&vertices[vIndex],&vertices[vIndex+1],&vertices[vIndex+2]);
					vIndex+=3;
				}
			break;
			case 'f':
				c=fgetc(f);
				b=2;
				while(c!='\n' && b>0)
				{
					b=fscanf(f,"%d//%d",&faces[fIndex],&faces[fIndex+1]); //return the number of successfully matched and assigned input items; 
					if(b==2)
					{
						fIndex+=2; //count the number of items in "faces" array.
						c=fgetc(f);
						faceElements[eIndex]++; //eIndex is the current face."faceElements" counts the number of vertices the face with index eIndex has.
					}
				}
				current_object->addFace(eIndex);
				eIndex++; //move the index to the next face.
			break;
			default:
				while(c!='\n' && c!=EOF) //Ignore g and comments
					c=fgetc(f);
			break;
		}
		if(c!=EOF)
			c=fgetc(f);
	}
	objects_vect.push_back(current_object);
	fclose(f);
	printf("indices %d %d %d %d %d\n",vIndex,vnIndex,fIndex,eIndex,line);
}

void calculateCenter(void)
{
	int i = 0, x, y, z, num_of_vert;
	vector<int> faces_vect;
	vector<Objects*>::iterator it;
	for(it = objects_vect.begin(); it != objects_vect.end(); ++it)
	{
		x = y = z = num_of_vert = 0;
		faces_vect = (*it)->getFaces();
		vector<int>::iterator it2;
		for(it2 = faces_vect.begin(); it2 != faces_vect.end(); ++it2)
		{
			for(unsigned int j = 0; j < faceElements[*it2]*2; j+=2)
			{
				x += vertices[(faces[i+j]-1)*3];
				y += vertices[(faces[i+j]-1)*3+1];
				z += vertices[(faces[i+j]-1)*3+2];
				num_of_vert++;
			}
			i += faceElements[*it2]*2;
		}
		x = x/num_of_vert;
		y = y/num_of_vert;
		z = z/num_of_vert;
		(*it)->setCenter(x, y, z);
	}
}

void init(void)
{
	left_mouse_down = false;
	right_mouse_down = false;
	middle_mouse_down = false;
	fov = 60.0;
	mode = Camera;
	mouse_x = mouse_y = numeric_limits<int>::max();
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(fov, 1.0f, 2.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);

	//--------------------------------------------LIGHT----------------------------------------//
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);
    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_diffuse[] = { 0.0, 0.5, 0.5, 1.0 };
    GLfloat light_specular[] = { 0.0, 0.0, 0.5, 1.0 };
    GLfloat light_directional[] = { 0.0, -1.0, -1.0 };
    GLfloat light_position[] = { -1.0, 1.0, -1.0, 0.0 };

    GLfloat object_ambient[] = { 0.3, 0.4, 0.5, 1.0 };
    GLfloat object_diffuse[] = { 0.0, 0.6, 0.7, 1.0 };
    GLfloat object_specular[] = { 0.0, 0.0, 0.8, 1.0 };
    GLfloat object_shine[] = { 5 };
	GLfloat mat_shin[] = {5.0};
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
	reader();
	calculateCenter();
}

void draw(GLenum drawing_mode)
{	
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

	int i = 0;
	vector<int> faces_vect;
	vector<Objects*>::iterator it;
	for(it = objects_vect.begin(); it != objects_vect.end(); ++it)
	{
		faces_vect = (*it)->getFaces();
		if((*it)->isSelected()){
			printf("HERE\n");
			glColor3f(1,0,0);
		}
		if(drawing_mode == GL_SELECT)
			glLoadName((*it)->getId());

		glPushMatrix();
			//Save current matrix & Load Identity
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			//Translate the object
			glTranslatef((*it)->getTranslationX(), (*it)->getTranslationY(), (*it)->getDepth());

			//Multiply by saved matrix
			glMultMatrixf(mview);

			//Rotate the object
			vector<float> center_vect = (*it)->getCenter();
			glTranslatef(center_vect.at(0), center_vect.at(1), center_vect.at(2));
			glRotatef((*it)->getAngleX(), 1.0, 0.0, 0.0);
			glRotatef((*it)->getAngleY(), 0.0, 1.0, 0.0);
			glTranslatef(-center_vect.at(0), -center_vect.at(1), -center_vect.at(2)); //Translate to origin

			vector<int>::iterator it2;
			for(it2 = faces_vect.begin(); it2 != faces_vect.end(); ++it2)
			{
				glBegin(GL_POLYGON);
					for(unsigned int j = 0; j < faceElements[*it2]*2; j+=2)
					{
						glNormal3f(normals[(faces[i+j+1]-1)*3],normals[(faces[i+j+1]-1)*3+1],normals[(faces[i+j+1]-1)*3+2]);
						glVertex3f(vertices[(faces[i+j]-1)*3],vertices[(faces[i+j]-1)*3+1],vertices[(faces[i+j]-1)*3+2]);
					}
				glEnd();
				i += faceElements[*it2]*2;
			}
		glPopMatrix();
		if((*it)->isSelected()){
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
	}
}

float scale_x, scale_y, scale_z;
void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
		glScalef(scale_x, scale_y, scale_z);
		glTranslatef(translate_right_scene, translate_upward_scene, 0);
		glRotatef(x_rotation_angle, 0.0, 1.0, 0.0);
		glRotatef(y_rotation_angle, 1.0, 0.0, 0.0);
		draw(GL_RENDER);
	glPopMatrix();
	glFlush();
}

void processHits(int hits)
{
	GLuint minZ, *ptr, names, *ptrNames;

	ptr = (GLuint*) selectBuf; 
	minZ = 0xffffffff;
	for(int i = 0; i < hits; i++)
	{
		names = *ptr;
		ptr++;
		if(*ptr < minZ)
		{
			minZ = *ptr;
			ptrNames = ptr + 2;
		}
		ptr += names + 2; //Go for the next hit
	}
	if(left_mouse_down){
		hitObjects.push_back(*ptrNames);
		(objects_vect.at(*ptrNames))->selected(true);
		printf("Object %d will be selected\n",*ptrNames);
	}
	else if(right_mouse_down){
		(objects_vect.at(*ptrNames))->selected(false);
		for(unsigned int i = 0; i < hitObjects.size(); i++){
			if(hitObjects.at(i) == *ptrNames){
				hitObjects.erase(hitObjects.begin() + i);
				printf("Object %d will not be selected\n",*ptrNames);
				break;
			}
		}
	}
	glutPostRedisplay();
}

void startPicking(int x, int y)
{
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
		gluPerspective(fov, 1.0f, 2.0, 200.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); //Add Global&Camera modes chaings
			glScalef(scale_x, scale_y, scale_z);
			glTranslatef(translate_right_scene, translate_upward_scene, 0);
			glRotatef(x_rotation_angle, 0.0, 1.0, 0.0);
			glRotatef(y_rotation_angle, 1.0, 0.0, 0.0);
			draw(GL_SELECT);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();
	if((hits = glRenderMode(GL_RENDER)) != 0)
	{
		processHits(hits);
	}
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	string windowName = "AMAZING 3D MODELING - ";
	switch(key)
	{
		//Esc
		case 27: 
			exit(0);
			break;
		//Backspace
		case ' ':
			printf("here\n");
			if(mode == Camera)
			{
				mode = Global;
				windowName += "GLOBAL";
			}
			else
			{
				mode = Camera;
				windowName += "CAMERA";
			}
			break;
		case 'p':
			if(mode != Picking)
			{
				mode = Picking;
				windowName += "PICKING";
			}
			else
			{
				mode = Camera;
				windowName += "CAMERA";
			}
			break;
		case 'a':
			if(mode == Picking){
				allPressed = !allPressed;
				windowName += (allPressed ? "PICKING ALL" : "NOT PICKING ANYTHING");
			}
			break;
		default:
			break;
	}
	glutSetWindowTitle(windowName.c_str());
}

void mouse(int button, int state, int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	if(button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			left_mouse_down = true;
			if(mode == Picking)
				startPicking(x, y);
		}
		else
			left_mouse_down = false;
	}
	else if(button == GLUT_RIGHT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			right_mouse_down = true;
			if(mode == Picking){
				startPicking(x, y);
			}
		}
		else
			right_mouse_down = false;
	}
	else
	{
		if(state == GLUT_DOWN)
		{
			middle_mouse_down = true;
			if(mode == Picking)
				startPicking(x, y);
		}
		else
			middle_mouse_down = false;
	}

	/*if(state == GLUT_UP && !middle_mouse_down && !right_mouse_down && !left_mouse_down)
		hit_object = -1;*/
}

void motion(int x, int y)
{
	int difference_x = mouse_x - x;
	int difference_y = mouse_y - y;
	float angle_x = -1.0 * float(mouse_x - x) * x_rotation_units;
	float angle_y = -1.0 * float(mouse_y - y) * y_rotation_units;
	if(left_mouse_down)
	{
		if(mode == Camera)
		{
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			x_camera_angle += angle_x;
			y_camera_angle += angle_y;
			if(x_camera_angle> 360 || x_camera_angle < -360)
				x_camera_angle = 0;
			if(y_camera_angle> 360 || y_camera_angle < -360)
				y_camera_angle = 0;
			glRotatef(angle_x, 0.0, 1.0, 0.0);
			glRotatef(angle_y, 1.0, 0.0, 0.0);
			glMultMatrixf(mview);
		}
		else if(mode == Global)
		{
			x_rotation_angle += angle_x;
			y_rotation_angle += angle_y;
			if(x_rotation_angle> 360 || x_rotation_angle < -360)
				x_rotation_angle = 0;
			if(y_rotation_angle> 360 || y_rotation_angle < -360)
				y_rotation_angle = 0;
		}
		else
		{
			if(allPressed){
				for(unsigned int i = 0; i < objects_vect.size(); i++){
					objects_vect.at(i)->updateAngles(angle_x, angle_y);
				}
			}
			for(unsigned int i = 0; i < hitObjects.size(); i++){
				objects_vect.at(hitObjects.at(i))->updateAngles(angle_x,angle_y);
			}
		}
		glutPostRedisplay();
	}
	if(right_mouse_down)
	{
		if(mode == Camera)
		{
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			translate_forward_camera += difference_y*translate_forward_units;
			glLoadIdentity();
			glTranslatef(0, 0, difference_y*translate_forward_units);
			glMultMatrixf(mview);
		}
		else if(mode == Global)
		{
			scale_x += difference_y*scale_units;
			scale_y += difference_y*scale_units;
			scale_z += difference_y*scale_units;
			if(scale_x < 0.01)
			{
				scale_x = scale_y = scale_z = 0.01;
			}
		}
		else
		{
			if(allPressed){
				for(unsigned int i = 0; i < objects_vect.size(); i++){
					objects_vect.at(i)->updateDepth(difference_y*translate_forward_units);
				}
			}
			for(unsigned int i = 0; i < hitObjects.size(); i++){
				objects_vect.at(hitObjects.at(i))->updateDepth(difference_y*translate_forward_units);
			}
		}
		glutPostRedisplay();
	}
	if(middle_mouse_down)
	{
		if(mode == Camera)
		{
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			translate_upward_camera += -difference_y*translate_upward_units;
			translate_right_camera += difference_x*translate_right_units;
			glLoadIdentity();
			glTranslatef(difference_x*translate_right_units, -difference_y*translate_upward_units, 0);
			glMultMatrixf(mview);
		}
		else if(mode == Global)
		{
			translate_upward_scene += difference_y*translate_upward_units;
			translate_right_scene += -difference_x*translate_right_units;
		}
		else
		{
			if(allPressed){
				for(unsigned int i = 0; i < objects_vect.size(); i++){
					objects_vect.at(i)->updateTranslation(-difference_x*translate_right_units, difference_y*translate_upward_units);
				}
			}
			for(unsigned int i = 0; i < hitObjects.size(); i++){
				objects_vect.at(hitObjects.at(i))->updateTranslation(-difference_x*translate_right_units, difference_y*translate_upward_units);
			}
		}
		glutPostRedisplay();
	}

	mouse_x = x;
	mouse_y = y;
}

int main(int argc, char **argv)
{
	x_camera_angle = 0;
	y_camera_angle = 0;
	translate_forward_scene = 0;
	translate_forward_camera = -20;
	translate_right_scene = 0;
	translate_right_camera = 0;
	translate_upward_scene = 0;
	translate_upward_camera = 0;
	window_size_x = 600;
	window_size_y = 500;
	x_rotation_units = 180.0/(float)window_size_x;
	y_rotation_units = 180.0/(float)window_size_y;
	translate_forward_units = 5.0/(float)window_size_y;
	translate_upward_units = 5.0/(float)window_size_y;
	translate_right_units = 5.0/(float)window_size_x;
	scale_units = 1.0/(float)window_size_y;
	scale_x = scale_y = scale_z = 1.0;
	x_rotation_angle = 0;
	y_rotation_angle = 0;
	glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(window_size_x, window_size_y);
    glutInitWindowPosition(150, 150);
    glutCreateWindow("AMAZING 3D MODELING");
    init();
    glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
    glutMainLoop();

    return 0;
}
