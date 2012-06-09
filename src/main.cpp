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
Vect PV,SunVect;
bool ViewOnEarth;
GLuint texture[1];
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
		for( float alpha = 0; alpha <= M_PI; alpha += M_PI/16 )
			for( float beta = 0-0.0001; beta <=2* M_PI; beta += M_PI/16 )
			{
				Sphere.push_back(R * sin(beta) * sin(alpha));
				Sphere.push_back(R * cos(beta));
				Sphere.push_back(R * sin(beta) * cos(alpha));
				Sphere.push_back(R * sin(beta + M_PI/16) * sin(alpha + M_PI/16));
				Sphere.push_back(R * cos(beta + M_PI/16));
				Sphere.push_back(R * sin(beta + M_PI/16) * cos(alpha + M_PI/16));	
				SphereDotsCount +=2;
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
		angle =  t * 24.0 * 60.0 * 60.0 * 7.292115*10e-6 / 2 ;
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
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_TEXTURE_GEN_S);
	//glEnable(GL_TEXTURE_GEN_T);
	glLoadIdentity();
	ReinitCamera();
	glColor3f(0.0,0.0,1.0);
	glVertexPointer(3,GL_FLOAT,0,Earth.ptr());
	glDrawArrays(GL_TRIANGLE_STRIP,0,Earth.Count());
	glColor3f(1.0,1.0,0.0);
	glVertexPointer(3,GL_FLOAT,0,Sun.ptr());
	glDrawArrays(GL_TRIANGLE_STRIP,0,Sun.Count());
	glColor3f(1.0,1.0,1.0);
	
	glBegin(GL_POINTS);
		glVertex3f(Integr->PhaseVect()[V_Y],Integr->PhaseVect()[V_Z],Integr->PhaseVect()[V_X]);
		glColor3f(1.0,0.0,0.0);
		glVertex3f(Earth.G.V[V_X],Earth.G.V[V_Y],Earth.G.V[V_Z]);
	glEnd();
	glColor3f(1.0,0.0,0.0);
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
		Angle = -(double)dX / kx / 2;
		Q = Quat(cos(Angle),Vect(0,sin(Angle),0));
		Qm1 = Quat(cos(Angle),Vect(0,-sin(Angle),0));
		rotVect = Q * rotVect * Qm1;
		Angle = -(double)dY / ky / 2;
		Q = Quat(cos(Angle),Vect(0,sin(Angle),0) % rotVect.V );
		Qm1 = Quat(cos(Angle),Vect(0,-sin(Angle),0)% rotVect.V );
		rotVect = Q * rotVect * Qm1;
	//rotVect.V.print();
	//printf("%f\n",(double)Angle);
	//glLoadIdentity();
	//ReinitCamera();
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
	glViewport(0,0,Width,Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho (0, W, H, 0, -1.0f, 100.0f);
	gluPerspective(70.0, Width/(float)Height, 0.01, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ReinitCamera();
}

void InitGL(int Width, int Height)
{
	AUX_RGBImageRec *texture;
	texture = auxDIBImageLoad("earth_1.bmp");
	
    	glEnable(GL_TEXTURE_2D);
	glGenTextures(1,&texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture->sizeX, texture->sizeY,0,GL_RGB,GL_UNSIGNED_BYTE,texture->data);
	delete texture;	
	glEnable(GL_AUTO_NORMAL);
	glClearColor( 0,0,0.07,0 );
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClearDepth(1.0);
	glDepthFunc(GL_LESS);
    	//glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
    	//glDisable(GL_ALPHA_TEST);
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
	if(Integr->getT()<Integr->getTk())
{
	usleep(10*1000);
	if(timer >= 5)
	{
		Integr->NextStep();
		timer = 0;
		PV = Integr->PhaseVect()*(149597870.66*10e-6);
		Earth.Offset(PV[V_Y],PV[V_Z],PV[V_X]);
		Earth.Rotate(Integr->getStep());
	}	
}
	Vect GVect = Earth.IG.V * (1 / Earth.IG.V.Length()) * ( Earth.IG.l * 10e-6 + Earth.IG.V.Length() );
//	Earth.IG.V.print();
	SunVect = PV + GVect;
	GVect = GVect * (1 / GVect.Length()) *( Earth.IG.l * 10e-6 * (-1));
	long double an =acos( GVect * SunVect / (GVect.Length() * SunVect.Length()));
	long double shadow = cos( M_PI/2 - an ) * GVect.Length()/cos(an);
	//printf("%f\n",(double)an);
	if(an > M_PI/2 && an<M_PI)
	printf("sun=%f,gnomon=%f,shadow=%f\n",(double)SunVect.Length(),(double)GVect.Length(),(double) shadow);
	glutPostRedisplay();
} 

int main(int argc, char **argv)
{
	
	kepl = new KeplerModel;
	Integr = new DPIntegrator(kepl,0,365,10e-20);
	rotVect = Quat(0,Vect(0,0,20.0));
	ViewVect = Quat(0,Vect(0,1,0));
	Angle = 12.5/180.0*M_PI;
	Q = Quat(cos(Angle),Vect(0,0,sin(Angle)));
	Qm1 = Quat(cos(Angle),Vect(0,0,-sin(Angle)));
	ViewVect = Q * ViewVect * Qm1;
	printf("GL inititialization start...\n");
	Earth = Globe(6378.137*10e-6,10,M_PI/2,M_PI/2);//(6378.137/149597870.66);
	Sun = Globe(695500.0*10e-6,0,0,0);
	glutInit(&argc, argv);
	PV = Vect(6);	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(100, 100);

	window = glutCreateWindow("OpenGLTutor");
	int window2 = glutCreateWindow("Graphs");
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
