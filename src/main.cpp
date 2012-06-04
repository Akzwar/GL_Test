#include <stdio.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>
#include <math.h>
#include "../../DPIntegrator/src/dpintegrator.h"
#include <vector>
#define W 1280
#define H 1050

using namespace std;

int window;
int SphereDotsCount = 0;
GLfloat Cube[ 8 * 3 ];
vector<GLfloat> Sphere;
Model* kepl;
DPIntegrator* Integr;

void DrawGLScene()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,1.0,0.0);		
	glEnd();
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(1.0,0.0,0.0);
	glEnd();
	glColor3f(0.0,0.0,1.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,0.0,1.0);
	glEnd();
	glDrawArrays(GL_POINTS,0,Sphere.size()-1);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_POINTS);
		glVertex3f(Integr->PhaseVect()[V_X],Integr->PhaseVect()[V_Y],Integr->PhaseVect()[V_Z]);
	glEnd();
    glutSwapBuffers(); 
	
}

bool IsDrag;

int prevX, prevY;
void onMouseClick(int button, int state, int x, int y)
{
	if( button == GLUT_LEFT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			IsDrag = true;
			prevX = x;
			prevY = y;
		}
		if( state == GLUT_UP )
			IsDrag = false;
	}
}


long double Angle;
Quat rotVect,Q,Qm1;

void onMouseMove( int X, int Y )
{
if( IsDrag )
{
	int kx = 900;
	int ky = 1500;
	int dX = X - prevX;
	int dY = Y - prevY;
		Angle = -(double)dX / kx;
		Q = Quat(cos(Angle),Vect(0,sin(Angle),0));
		Qm1 = Quat(cos(Angle),Vect(0,-sin(Angle),0));
		rotVect = Q * rotVect * Qm1;
		Angle = -(double)dY / ky;
		Q = Quat(cos(Angle),Vect(0,sin(Angle),0) % rotVect.V );
		Qm1 = Quat(cos(Angle),Vect(0,-sin(Angle),0)% rotVect.V );
		rotVect = Q * rotVect * Qm1;
	//rotVect.V.print();
	//printf("%f\n",(double)Angle);
	glLoadIdentity();
	gluLookAt(rotVect.V[V_X], rotVect.V[V_Y], rotVect.V[V_Z], 0,0,0, 0,1,0);
	prevX = X;
	prevY = Y;
}
}

void keyPressed(unsigned char key, int x, int y)
{
	if( key == 's' )
		rotVect.V = rotVect.V * 1.1;
	if( key == 'w' )
		rotVect.V = rotVect.V * 0.9;	
	glLoadIdentity();
	gluLookAt(rotVect.V[V_X], rotVect.V[V_Y], rotVect.V[V_Z], 0,0,0, 0,1,0);

}

void ReSizeGLScene(int Width, int Height)
{
	glViewport(0,0,Width,Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho (0, W, H, 0, -1.0f, 100.0f);
	gluPerspective(70.0, W/H, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(rotVect.V[V_X], rotVect.V[V_Y], rotVect.V[V_Z], 0,0,0, 0,1,0);
}

void InitGL(int Width, int Height)
{
	glClearColor( 0,0,0.07,0 );
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClearDepth(1.0);
	//glDepthFunc(GL_LESS);
    	//glDepthMask(GL_FALSE);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
    	//glDisable(GL_ALPHA_TEST);
    	//glEnable(GL_TEXTURE_2D);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glShadeModel(GL_FLAT);

	//glGenTextures(1, &gl_depth_tex);
	//glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glGenTextures(1, &gl_rgb_tex);
	//glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ReSizeGLScene(Width, Height);
	//glVertexPointer(3,GL_FLOAT,0,&Cube);
	glVertexPointer(3,GL_FLOAT,0,&Sphere[0]);
	glEnableClientState(GL_VERTEX_ARRAY);
	glPointSize(3.0);

}
int timer = 0;
void onIdle(void)
{
	timer ++;
	usleep(10*1000);
	if(timer >= 5)
	{
		Integr->NextStep();
		timer = 0;
	}
		glutPostRedisplay();
} 

int main(int argc, char **argv)
{
	kepl = new KeplerModel;
	Integr = new DPIntegrator(kepl,0,100,10e-8);
	Angle = 0.1;
	rotVect = Quat(0,Vect(0,0,20.0));
	Cube[23] = -1;
	Cube[22] = 1;
	Cube[21] = -1;
	Cube[20] = 1;
	Cube[19] = 1;
	Cube[18] = -1;
	Cube[17] = 1;
	Cube[16] = -1;
	Cube[15] = -1;
	Cube[14] = -1;
	Cube[13] = -1;
	Cube[12] = -1;
	Cube[11] = -1;
	Cube[10] = -1;
	Cube[9] = 1;
	Cube[8] = 1;
	Cube[7] = -1;
	Cube[6] = 1;
	Cube[5] = 1;
	Cube[4] = 1;
	Cube[3] = 1;
	Cube[2] = -1;
	Cube[1] = 1;
	Cube[0] = 1;
	GLfloat R = 10.0;
	for( float alpha = 0; alpha < 2*M_PI; alpha += M_PI/16 )
		for( float beta = 0; beta < M_PI; beta += M_PI/16 )
		{
			//intf("%f\n",(i*R*10+i*10)*3);
			Sphere.push_back(R * sin(beta) * sin(alpha));
			Sphere.push_back(R * cos(beta));
			Sphere.push_back(R * sin(beta) * cos(alpha));			
		}
	printf("GL inititialization start...\n");

	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(0, 0);

	window = glutCreateWindow("OpenGLTutor");

	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&onIdle);
	glutReshapeFunc(&ReSizeGLScene);
	glutKeyboardFunc(&keyPressed);
	glutMouseFunc(onMouseClick);
	glutMotionFunc(onMouseMove);
	InitGL(W, H);
	printf("GL Init done...\n");
	glutMainLoop();
	delete Integr;
	delete kepl;
	return 0;
}
