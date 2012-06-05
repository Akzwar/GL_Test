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
GLfloat Cube[ 8 * 3 ];
Model* kepl;
DPIntegrator* Integr;
Quat rotVect,Q,Qm1,ViewVect;
Vect PV;
bool ViewOnEarth;
struct Gnomon
{
	Vect V;
	float l;
};
class Globe
{
 private:
	vector<float> InitSphere;
	int SphereDotsCount;
	vector<float> Sphere;
 public:
	Gnomon IG,G;
	Globe(){}
	Globe(float R, float _l, float _alpha, float _beta)
	{
		G.V = Vect(R*sin(_beta)*sin(_alpha),R*cos(_beta),R*sin(_beta)*cos(_alpha));
		G.l = _l;
		IG  = G;
		SphereDotsCount = 0;
		for( float alpha = 0; alpha <= 2*M_PI; alpha += M_PI/16 )
			for( float beta = 0; beta <= M_PI; beta += M_PI/16 )
			{
				Sphere.push_back(R * sin(beta) * sin(alpha));
				Sphere.push_back(R * cos(beta));
				Sphere.push_back(R * sin(beta) * cos(alpha));			
				SphereDotsCount ++;
			}
		InitSphere = Sphere;
		//printf("%d\n",SphereDotsCount);
	}
	void Offset(float X, float Y, float Z)
	{
		Sphere.clear();
		G.V[V_X] = IG.V[V_X] + X;
		G.V[V_Y] = IG.V[V_Y] + Y;
		G.V[V_Z] = IG.V[V_Z] + Z; 
		for( int i = 0; i < SphereDotsCount*3; i+=3 )
		{		
			Sphere.push_back( InitSphere[i] + X );
			Sphere.push_back( InitSphere[i+1] + Y );
			Sphere.push_back( InitSphere[i+2] + Z );
		}
	}
	void Rotate(long double t)
	{
		Quat res,q,qm1,tmp;	
		long double angle;
		angle = t*24*60*60*7.292115*10e-5;
		q = Quat(cos(angle),Vect(0,sin(angle),0));
		qm1 = Quat(cos(angle), Vect(0,-sin(angle),0));
		tmp = Quat(0,Vect(IG.V));
		res = q * tmp * qm1; 
		IG.V = res.V;
		for( int i = 0; i < SphereDotsCount*3; i+=3 )
		{
			tmp = Quat(0,Vect(InitSphere[i],InitSphere[i+1],InitSphere[i+2]));
			res = q * tmp * qm1;
			InitSphere[i] = res.V[V_X];
			InitSphere[i+1] = res.V[V_Y];
			InitSphere[i+2] =res.V[V_Z];
		}
	}
	void* ptr(){return &Sphere[0];}
	int Count(){return SphereDotsCount;}
};

Globe Earth,Sun;

void ReinitCamera()
{	
	if(ViewOnEarth)	
	gluLookAt(PV[V_Y]+rotVect.V[V_X], PV[V_Z]+rotVect.V[V_Y], PV[V_X]+rotVect.V[V_Z],
 		PV[V_Y],PV[V_Z],PV[V_X], 
		ViewVect.V[V_X],ViewVect.V[V_Y],ViewVect.V[V_Z]);
	else
	gluLookAt(rotVect.V[V_X], rotVect.V[V_Y], rotVect.V[V_Z],
 		0,0,0, 
		ViewVect.V[V_X],ViewVect.V[V_Y],ViewVect.V[V_Z]);
}

void DrawGLScene()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   /* 
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,10.0,0.0);		
	glEnd();
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(10.0,0.0,0.0);
	glEnd();
	glColor3f(0.0,0.0,1.0);
	glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,0.0,10.0);
	glEnd();
*/
	//glPushMatrix();
	glLoadIdentity();
	ReinitCamera();
	glColor3f(0.0,0.0,1.0);
	glVertexPointer(3,GL_FLOAT,0,Earth.ptr());
	glDrawArrays(GL_POINTS,0,Earth.Count());
	glColor3f(1.0,1.0,0.0);
	glVertexPointer(3,GL_FLOAT,0,Sun.ptr());
	glDrawArrays(GL_POINTS,0,Sun.Count());
	//glPopMatrix();
	glColor3f(1.0,1.0,1.0);
	
	glBegin(GL_POINTS);
		glVertex3f(Integr->PhaseVect()[V_Y],Integr->PhaseVect()[V_Z],Integr->PhaseVect()[V_X]);
		glColor3f(1.0,0.0,0.0);
		glVertex3f(Earth.G.V[V_X],Earth.G.V[V_Y],Earth.G.V[V_Z]);
	glEnd();
    glutSwapBuffers(); 
	
}

bool IsDrag_Middle;
bool IsDrag_Left;

int prevX, prevY;
void onMouseClick(int button, int state, int x, int y)
{
	if( button == GLUT_LEFT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			IsDrag_Left = true;
			prevX = x;
			prevY = y;
		}
		if( state == GLUT_UP )
			IsDrag_Left = false;
	}
	if( button == GLUT_MIDDLE_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			IsDrag_Middle = true;
			prevX = x;
			prevY = y;
		}
		if( state == GLUT_UP )
			IsDrag_Middle = false;
	}
}


long double Angle;
Vect Center(3);
void onMouseMove( int X, int Y )
{
if( IsDrag_Left )
{
	int kx = 900;
	int ky = 15000;
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
	ReinitCamera();
	prevX = X;
	prevY = Y;
}
if( IsDrag_Middle )
{
	//int dX = X - prevX;
	//int dY = Y - prevY;
	
	//glLoadIdentity();
	
}

}

void keyPressed(unsigned char key, int x, int y)
{
	if( key == 's' )
		rotVect.V = rotVect.V * 1.1;
	if( key == 'w' )
		rotVect.V = rotVect.V * 0.9;	
	if( key == 'e' )
		if(ViewOnEarth)
			ViewOnEarth = false;
		else
			ViewOnEarth = true;
	glLoadIdentity();
	ReinitCamera();
}

void ReSizeGLScene(int Width, int Height)
{
	glViewport(0,0,W,H);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho (0, W, H, 0, -1.0f, 100.0f);
	gluPerspective(70.0, Width/Height, 0.01, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ReinitCamera();
}

void InitGL(int Width, int Height)
{
	glClearColor( 0,0,0.07,0 );
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClearDepth(1.0);
	glDepthFunc(GL_LESS);
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
		printf("%f\n",(double)Integr->getStep());
		PV = Integr->PhaseVect()*(149597870.66/6378.137*0.01);
		Earth.Offset(PV[V_Y],PV[V_Z],PV[V_X]);
		Earth.Rotate(Integr->getStep());
	}
	glutPostRedisplay();
} 

int main(int argc, char **argv)
{
	
	kepl = new KeplerModel;
	Integr = new DPIntegrator(kepl,0,100,10e-20);
	rotVect = Quat(0,Vect(0,0,20.0));
	ViewVect = Quat(0,Vect(0,1,0));
	Angle = 12.5/180.0*M_PI;
	Q = Quat(cos(Angle),Vect(0,0,sin(Angle)));
	Qm1 = Quat(cos(Angle),Vect(0,0,-sin(Angle)));
	ViewVect = Q * ViewVect * Qm1;
	printf("GL inititialization start...\n");
	Earth = Globe(0.01,10,M_PI/2,M_PI/2);//(6378.137/149597870.66);
	Sun = Globe(695500.0/6378.137*0.01,0,0,0);
	glutInit(&argc, argv);
	PV = Vect(6);	
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
