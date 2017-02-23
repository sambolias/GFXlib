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
		
		//shader program functions
		GLuint compileShader(GLenum shaderType, const string source)
		{
			GLuint shader = glCreateShader(shaderType);

			const char * sourceArray[1] = { source.c_str() };	//needs to match const GLchar *
			glShaderSource(shader, 1, sourceArray, NULL);		
			glCompileShader(shader);

			GLint compileResults;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResults);

			if (compileResults == 0)
			{
				//this means there was an error
				//need to do something here
				//see other project for how to log
			}

		}
		
		
		GLuint compileShaderProgram(const string &vsSource, const string &fsSource)
		{
			GLuint program = glCreateProgram();

			if (program == 0) 
			{
				//should probably throw program creation failed error
			}

			GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
			GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);

			if (vs == 0 || fs == 0)	//if either failed to compile
			{	//clean up mess
				glDeleteShader(vs);
				glDeleteShader(fs);
				glDeleteProgram(program);
				//maybe throw, but return for now
				return 0;
			}

			glAttachShader(program, vs);
			glDeleteShader(vs);	//no longer needed

			glAttachShader(program, fs);
			glDeleteShader(fs);	//same, RAII

			glLinkProgram(program);

			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);	//check similar to shaders

			if (linkStatus == 0)
			{
				//needs to throw or return 0
				//so that nobody trys to use bad program
				//I need to make overall exception strategy first
			}

			return program;

		}



		//vertex shader for loaded texture
		const string vs = R"(
			attribute vec4 aPos;
			attribute vec2 aCoord;
			varying vec2 vCoord;

			void main()
			{	
				gl_Position = aPos;
				vCoord = aCoord;
			}
		)";

		//fragment shader for loaded texture
		const string fs = R"(
			precision mediump float;
			varying vec2 vCoord;
			uniform sampler2D sTex;

			void main()
			{
				gl_FragColor = texture2D(sTex, vCoord);	
			}	
		)";
		//need to do a lot of refactoring, put stuff in header once this works
		GLuint _program;
		GLuint _positionAttribute;
		GLuint _colorAttribute;
		GLuint _modelUniform;
		GLuint _viewUniform;
		GLuint _projUniform;
		int texWidth, texHeight;
		//heres where the magic happens
	public:
		texture(const string &file)
		{
			_program = compileShaderProgram(vs, fs);
			_positionAttribute = glGetAttribLocation(_program, "aPosition");
			_colorAttribute = glGetAttribLocation(_program, "aColor");
			_modelUniform = glGetUniformLocation(_program, "uModelMatrix");
			_viewUniform = glGetUniformLocation(_program, "uViewMatrix");
			_projUniform = glGetUniformLocation(_program, "uProjMatrix");
		}

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

