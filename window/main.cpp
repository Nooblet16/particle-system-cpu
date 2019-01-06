#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>
#include <ctime>
#include <gl\glew.h>
#include <gl\glut.h>

const unsigned int window_w = 1024;
const unsigned int window_h = 1024;

const unsigned int width_of_net = 512;
const unsigned int height_of_net = 512;

float arrayA[width_of_net*height_of_net];
float arrayB[width_of_net*height_of_net];

int mouse_x, mouse_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float transfer_z = -3.0;

float movement = 0.0;

struct float3
{
	float x, y, z;
};

struct float4
{
	float x, y, z, w;
};

float3 make_float3(float X, float Y, float Z)
{
	float3 temporary = {X,Y,Z};
	return temporary;
}

float4 make_float4(float X, float Y, float Z, float W)
{
	float4 temporary = {X,Y,Z, W};
	return temporary;
}

float3 position[width_of_net][height_of_net];
float3 velocity[width_of_net][height_of_net];
float4 col[width_of_net][height_of_net];


void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		mouse_buttons |= 1<<button;
	} else if (state == GLUT_UP) {
		mouse_buttons = 0;
	}

	mouse_x = x;
	mouse_y = y;
	glutPostRedisplay();
}

void MouseListner(int x, int y)
{
	float dx, dy;
	dx = x - mouse_x;
	dy = y - mouse_y;

	if (mouse_buttons & 1) {
		rotate_x += dy * 0.2;
		rotate_y += dx * 0.2;
	} else if (mouse_buttons & 4) {
		transfer_z += dy * 0.01;
	}

	mouse_x = x;
	mouse_y = y;
}

void initialize()
{
#pragma omp parallel for
	for (int x=0;x<width_of_net;++x)
		for (int y=0;y<height_of_net;++y)
		{
			float u = x / (float) width_of_net + arrayA[y*width_of_net+x];
			float v = y / (float) height_of_net + arrayB[y*height_of_net+x];

			float frequency = 1.5f;
			float w = sin(u*frequency + movement) * cos(v*frequency + movement) * 1.0f;

			col[y][x] = make_float4(0,255,255,1);
			position[y][x] = make_float3(u, w, v);
			velocity[y][x] = make_float3(0.0, 0.0, 0.0);
		}
}

//Thanks to Egor for help with this code
void GenParticles()
{
	const float speed = 0.0009f;
	const float threshold = 0.4f;
#pragma omp parallel for
	for (int x=0;x<width_of_net;++x)
		for (int y=0;y<height_of_net;++y)
		{
			float u = x / (float) width_of_net;
			float v = y / (float) height_of_net;
			float xX = (mouse_x - (float)width_of_net/2-256)/(float)width_of_net/2;
			float yY = (mouse_y - (float)height_of_net/2-256)/(float)height_of_net/2;
			float dx = -position[y][x].x + xX;
			float dz = -position[y][x].z + yY;
			float length = sqrt(dx*dx+dz*dz);
			if (mouse_buttons==10)
			{
				velocity[y][x].x=0;
				velocity[y][x].z=0;
				dx = -position[y][x].x + u;
				dz = -position[y][x].z + v;
				length = sqrt(dx*dx+dz*dz);
				position[y][x].x+=dx/length*speed*10;
				position[y][x].z+=dz/length*speed*10;
			}
			else if (!(mouse_buttons & 4) && !(mouse_buttons & 6))
			{
				float normaliseX = dx/length*speed;
				float normaliseZ = dz/length*speed;
				velocity[y][x].x+=normaliseX;
				velocity[y][x].z+=normaliseZ;
				dx = velocity[y][x].x;
				dz = velocity[y][x].z;
				float maxvelocityocity = sqrt(dx*dx+dz*dz);
				if (maxvelocityocity>threshold)
				{
					velocity[y][x].x=dx/maxvelocityocity*threshold;
					velocity[y][x].z=dz/maxvelocityocity*threshold;
				}
				float green = (int)(255/(maxvelocityocity*51))/255.0f;
				if (green>=1.0f)
					green=1.0f;
				col[y][x] = make_float4(128/length/255.0f,green,1.0,0.1);
				if (position[y][x].x<-5.0f && velocity[y][x].x<0.0)
					velocity[y][x].x=-velocity[y][x].x;
				if (position[y][x].x>5.0f && velocity[y][x].x>0.0)
					velocity[y][x].x=-velocity[y][x].x;
				position[y][x].x+=velocity[y][x].x;
				position[y][x].z+=velocity[y][x].z;
			}
			else if (!(mouse_buttons & 4))
			{
				velocity[y][x].x=0;
				velocity[y][x].z=0;
				position[y][x].x+=dx/length*speed*10;
				position[y][x].z+=dz/length*speed*10;
				col[y][x] = make_float4(1.0f/length,1.0f/length, 1.0f, 20);
			}
			float frequency = 1.5f;
			float w = sin(u*frequency + movement) * cos(v*frequency + movement) * 1.0f;
			position[y][x].y=w;
		}
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GenParticles();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0, 0.0, transfer_z);
	glRotatef(90.0, 1.0, 0.0, 0.0);

	glBegin(GL_POINTS);
	for (int x=0;x<width_of_net;++x)
		for (int y=0;y<height_of_net;++y)
		{
			glColor4f(col[y][x].x, col[y][x].y, col[y][x].z, col[y][x].z);
			glVertex3f(position[y][x].x, position[y][x].y, position[y][x].z);
		}
		glEnd();

		glutSwapBuffers();
		glutPostRedisplay();

		movement += 0.01;
}

void initializeGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(window_w, window_h);
	glutCreateWindow("Particle Systemporary");
	glutDisplayFunc(display);
	glutMotionFunc(MouseListner);

	glewInit();
	if (! glewIsSupported("GL_VERSION_2_0 ")) {
		fprintf(stderr, "Proper OpenGL support missing.");
		fflush(stderr);
		exit(0);
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, window_w, window_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (GLfloat)window_w / (GLfloat) window_h, 0.01, 20.0);
}

int main(int argc, char** argv)
{
	initializeGL(argc, argv);

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(MouseListner);

	for (int i=0;i<height_of_net*width_of_net;++i)
		arrayA[i]=(rand()%50-50)/1000.0f;
	for (int i=0;i<height_of_net*width_of_net;++i)
		arrayB[i]=(rand()%50-50)/1000.0f;

	initialize();

	glutMainLoop();

	return 0;
}