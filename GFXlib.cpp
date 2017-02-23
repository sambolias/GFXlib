// GFXlib.cpp : Defines the entry point for the console application.
//

#include "GFXlib.h"



//default glut loop functions...need to find out how to store/replace
void defaultTimer(int)
{
	if (glutGetWindow())
	{
		glutPostRedisplay();
		glutTimerFunc(20, defaultTimer, 0);
	}
};

void defaultKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		glutDestroyWindow(glutGetWindow());
		break;
	default:
		break;
	}
}

void defaultDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutSwapBuffers();
	glutPostRedisplay();
}

class display	//this will create display and start glut loop
{				//must figure out how to pass functions to display
public:

	display()	//should this take title? or should it get set function like res, functions, etc will
	{
//		timerEvent = &defaultTimer;	//namespace problem? I don't get it
		title = "My GFX lib";
	}

	int winHeight = 3000, winWidth = 2000;	//need to find out how to init these for diff machines
	int refresh = 20;	//need to make sure this does what i think it does
	string title;

	void (*timerEvent)(int) = &defaultTimer;
	void (*draw)() = &defaultDraw;
	void(*keyboard)(unsigned char, int, int) = &defaultKeyboard;
	

	

public:

	void openDisplay(int *ac, char ** av)	//need to find out what these command line params are used for
	{

		glutInit(ac, av);	//at_EXIT_HACK for windows, research

		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);	//is there any reason to change these

		glutInitWindowSize(winWidth, winHeight);

		glutCreateWindow(title.c_str());

		glutDisplayFunc(defaultDraw);
		glutKeyboardFunc(defaultKeyboard);
		glutTimerFunc(refresh, timerEvent, 0);

		//how much harder would it to be to access OpenGL without freeglut and glew?
		//if I'm piggybacking on these libraries I have to accept all glutxxxxFunc binds

		glClearColor(0.f, 0.f, 0.f, 0.f);	//this needs to take variables
		glViewport(0, 0, winWidth, winHeight);	//this needs more customizable, maybe...prob not for 2d

		glMatrixMode(GL_PROJECTION);	//again prob good for 2d
		glLoadIdentity();

		//Init GLEW
		//is glew needed without 3d objects?
		gluPerspective(60.f, (GLfloat)winWidth / winHeight, 0.f, 50.f);
	//	glewInit();

		//run glut loop
		glutMainLoop();	



	}

	class texture	//this will load textures that can be drawn to display
	{				//should this hold shader program or display?


	};
};


int main(int argv, char **argc)
{
	display test;

	test.openDisplay(&argv,argc);

	std::cout << "It compiles\n";
	while(std::cin.get()!=10)
    return 0;
}

