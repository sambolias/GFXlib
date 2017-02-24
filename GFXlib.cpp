//GFXlib.cpp
//Sam Erie
//2/22/17
//start of my graphics library

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

//both classes badly need refactor
//need destructors, memory leaks aplenty!
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

		return shader;
	}


	GLuint compileShaderProgram(const string &vsSource, const string &fsSource)
	{
		GLuint program = glCreateProgram();

		if (program == 0)
		{
			//should probably throw program creation failed error
			return 0;
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
			uniform sampler2D uTex;

			void main()
			{
				gl_FragColor = texture2D(uTex, vCoord);	
			}	
		)";
	//need to do a lot of refactoring, put stuff in header once this works
	GLuint _program;

	GLuint _texture;
	GLuint _samplerLocation;
	GLuint _textureCoord;	//location or coordinates?

	GLuint _positionAttribute;
	GLuint _colorAttribute;
	
	int texWidth, texHeight;
	//heres where the magic happens

	//again really need to refactor, here is the vert and ind to draw texture on
	//need to fix these values to relate to resizing image, once i get this working
	//I am combining code from my summer project
	GLfloat h = 1.0f;	//figure out draw size
	GLfloat r = 1.0f;
	GLfloat w = 1.0f;
	GLfloat b = 1.0f;
	GLfloat Vertices[20] =
	{
		-h, w, 0.0f,
		0.0f, b,
		-h, -w, 0.0f,
		0.0f, 0.0f,
		r, -w, 0.0f,
		b, 0.0f,
		r, w, 0.0f,
		b, b
	};
	GLushort Indices[6] = { 0, 1, 2 ,0, 2, 3 };


public:
	bool isLoaded = false;	//this is messy, shouldn't be public
	texture() {}

	//scalar resize, should also have scalarx and scalary transforms, but with better names
	void resize(int width)
	{
		float scale = (float)width / (float)texWidth;
		texWidth *= scale;
		texHeight *= scale;
	}
	//rotate and other transform functions

	//proabably make textures possess their own coordinates
	//and layer info
	//then maybe animated sprites

	void load(const string &file)
	{
		//set up shaders

		//this needs error checking
		_program = compileShaderProgram(vs, fs);
		_positionAttribute = glGetAttribLocation(_program, "aPos");
		_textureCoord = glGetAttribLocation(_program, "aCoord");
		_samplerLocation = glGetUniformLocation(_program, "uTex");	//these names need fixed, not descriptive
		_colorAttribute = glGetAttribLocation(_program, "aColor");


		//load the texture from file

		//opengl part
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D, _texture);
		//stb_image.h part
		loadImage(file);

		isLoaded = true;
	}

	void drawTexture()
	{
		glUseProgram(_program);
		glVertexAttribPointer(_positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), Vertices);
		glVertexAttribPointer(_textureCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), Vertices + 3);

		glEnableVertexAttribArray(_positionAttribute);
		glEnableVertexAttribArray(_textureCoord);

		glActiveTexture(GL_TEXTURE0);	//this will be a conflict if drawing multiple textures
										//needs to be planned for
		glBindTexture(GL_TEXTURE_2D, _texture);

		glUniform1i(_samplerLocation, 0);	//also this

											//needed to overlap sprites - otherwise transparant background draw overwrites previously drawn background
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		/////////////////////////////////////////////////////

		glPushMatrix();
		//if tex class is nested in display
		//then this could default to quarter screen size minus half image size (center origin)
		//not sure why it needs quartered
		glViewport((int)(3000.f/4.f - texWidth/2.f), (int)(2000.f/4.f - texHeight/2.f), texWidth, texHeight);

		//this is the actual draw
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, Indices);

		glPopMatrix();
	}

private:


	//loads file using stbi_image free lib...needs error checking
	void loadImage(const string &file)
	{
		int width = 0, height = 0, channels = 0;

		unsigned char * img;

		stbi_set_flip_vertically_on_load(true);
		img = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		//save size to member variables
		texWidth = width;
		texHeight = height;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	}
};

///////////////////////////////


//need to find out best way to edit draw function
//possibly just binding new functions like this...
texture test;



void drawTest()
{
	//init tex for test
	if (!test.isLoaded)
	{
		test.load("test.jpg");
		test.resize(800);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	test.drawTexture();

	glutSwapBuffers();
	glutPostRedisplay();
}


/////////////////////////////////////////////

class display	//this will create display and start glut loop
{				//must figure out how to pass functions to display


	
public:

	display()	//should this take title? or should it get set function like res, functions, etc will
	{
		
		title = "My GFX lib";
		
	}

	int winHeight = 2000, winWidth = 3000;	//need to find out how to init these for diff machines
	int refresh = 20;	//need to make sure this does what i think it does
	string title;


	void (*timerEvent)(int) = &defaultTimer;
	void (*draw)() = &drawTest;
	void(*keyboard)(unsigned char, int, int) = &defaultKeyboard;
	

	

public:

	void openDisplay(int *ac, char ** av)	//need to find out what these command line params are used for
	{
		//init glut, error handling?
		glutInit(ac, av);	//at_EXIT_HACK for windows? research
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);	//is there any reason to change these
		glutInitWindowSize(winWidth, winHeight);
		glutCreateWindow(title.c_str());

		//Init GLEW
		GLenum error = glewInit();

		if (error != GLEW_OK)	//this like all other error checking
		{						//needs handled
			std::cout << "Found the problem\n";
		}


		glutDisplayFunc(draw);
		glutKeyboardFunc(defaultKeyboard);
		glutTimerFunc(refresh, timerEvent, 0);

		//how much harder would it to be to access OpenGL without freeglut and glew?
		//if I'm piggybacking on these libraries I have to accept all glutxxxxFunc binds


		//some of these calls are not necessary for 2d
		//need to research

					//R   G    B    A
		glClearColor(0.f, 0.f, 0.f, 0.f);	//this needs to take variables
	//	glViewport(0, 0, winWidth, winHeight);	//this needs more customizable, maybe...prob not for 2d

		glMatrixMode(GL_PROJECTION);	//again prob good for 2d
		glLoadIdentity();

				//field of view  - aspect ratio  - near distance- far distance
	//	gluPerspective(60.f, (GLfloat)winWidth / winHeight, 0.f, 30.f);
		

		//run glut loop
		glutMainLoop();	

	}

	
};



int main(int argv, char **argc)
{
	display test;

	test.openDisplay(&argv,argc);

	std::cout << "It compiles\n";
	while(std::cin.get()!=10)
    return 0;
}

